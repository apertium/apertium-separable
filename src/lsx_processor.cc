#include "lsx_processor.h"

#include <lttoolbox/compression.h>
#include <cstring>

LSXProcessor::LSXProcessor()
{
  escaped_chars.insert('[');
  escaped_chars.insert(']');
  escaped_chars.insert('{');
  escaped_chars.insert('}');
  escaped_chars.insert('^');
  escaped_chars.insert('$');
  escaped_chars.insert('/');
  escaped_chars.insert('\\');
  escaped_chars.insert('@');
  escaped_chars.insert('<');
  escaped_chars.insert('>');

  null_flush = false;
  dictionary_case = false;
  at_end = false;
  at_null = false;
}

void
LSXProcessor::load(FILE *input)
{
  fpos_t pos;
  if (fgetpos(input, &pos) == 0) {
    char header[4]{};
    if (fread(header, 1, 4, input) == 4 &&
        strncmp(header, HEADER_LTTOOLBOX, 4) == 0) {
      auto features = read_le<uint64_t>(input);
      if (features >= LTF_UNKNOWN) {
        throw std::runtime_error("FST has features that are unknown to this version of lttoolbox - upgrade!");
      }
    }
    else {
      // Old binary format
      fsetpos(input, &pos);
      // of course, lsx-comp would never generate this...
    }
  }

  // letters
  int len = Compression::multibyte_read(input);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<UChar32>(Compression::multibyte_read(input)));
    len--;
  }

  // symbols
  alphabet.read(input);
  word_boundary = alphabet("<$>"_u);
  word_boundary_s = alphabet("<$_>"_u);
  word_boundary_ns = alphabet("<$->"_u);
  any_char = alphabet("<ANY_CHAR>"_u);
  any_tag = alphabet("<ANY_TAG>"_u);

  len = Compression::multibyte_read(input);
  if (len > 0) {
    Compression::string_read(input); // name
    // there should only be 1 transducer in the file
    // so ignore any subsequent ones
    trans.read(input, alphabet);
    // but if there are 0, leave trans empty
    initial_state.init(trans.getInitial());
    all_finals = trans.getFinals();
  }
}

void
LSXProcessor::readNextLU(InputFile& input)
{
  vector<UString> parts = vector<UString>(3);
  int loc = 0; // 0 = blank, 1 = bound blank, 2 = LU
  bool box = false; // are we in a [ ] blank
  while(!input.eof())
  {
    UChar32 c = input.get();
    if ((unsigned int)c == U_EOF) {
        break;
    }
    if(null_flush && c == '\0')
    {
      at_end = true;
      at_null = true;
      break;
    }
    else if(c == '\\')
    {
      parts[loc] += c;
      c = input.get();
      parts[loc] += c;
    }
    else if(loc == 0 && box)
    {
      if(c == ']')
      {
        box = false;
      }
      parts[loc] += c;
    }
    else if(loc == 0 && c == '[')
    {
      c = input.get();
      if(c == '[')
      {
        loc = 1;
      }
      else
      {
        parts[loc] += '[';
        parts[loc] += c;
        if(c != ']')
        {
          box = true;
        }
        if(c == '\\')
        {
          parts[loc] += input.get();
        }
      }
    }
    else if(loc == 1 && c == ']')
    {
      c = input.get();
      if(c == ']')
      {
        c = input.get();
        if(c == '^')
        {
          loc = 2;
        }
        else
        {
          // this situation is invalid
          // but I like making parsers harder to break than required
          // by the standard
          parts[loc] += "]]"_u;
          parts[loc] += c;
        }
      }
      else
      {
        parts[loc] += ']';
        parts[loc] += c;
        if(c == '\\')
        {
          parts[loc] += input.get();
        }
      }
    }
    else if(loc == 0 && c == '^')
    {
      loc = 2;
    }
    else if(loc == 2 && c == '$')
    {
      break;
    }
    else
    {
      parts[loc] += c;
    }
  }
  if(input.eof())
  {
    at_end = true;
  }
  blank_queue.push_back(parts[0]);
  bound_blank_queue.push_back(parts[1]);
  lu_queue.push_back(parts[2]);
}

void
LSXProcessor::processWord(InputFile& input, UFILE* output)
{
  if(lu_queue.size() == 0)
  {
    readNextLU(input);
  }
  if(at_end && lu_queue.size() == 1 && lu_queue.back().size() == 0)
  {
    // we're at the final blank, no more work to do
    write(blank_queue.back(), output);
    blank_queue.pop_front();
    bound_blank_queue.pop_front();
    lu_queue.pop_front();
    return;
  }
  size_t last_final = 0;
  UString last_final_out;
  State s = initial_state;
  size_t idx = 0;
  bool firstupper = false;
  bool uppercase = false;
  while(s.size() > 0)
  {
    if(idx == lu_queue.size())
    {
      if(at_end)
      {
        break;
      }
      readNextLU(input);
    }
    UString lu = lu_queue[idx];
    if(lu.size() == 0)
    {
      break;
    }
    if (idx == 0 && !dictionary_case) {
      firstupper = iswupper(lu[0]);
      uppercase = lu.size() > 1 && iswupper(lu[1]);
    }
    for(size_t i = 0; i < lu.size(); i++)
    {
      if(lu[i] == '<')
      {
        size_t j = i+1;
        for(; j < lu.size(); j++)
        {
          if(lu[j] == '\\')
          {
            j++;
          }
          else if(lu[j] == '>')
          {
            j++;
            break;
          }
        }
        UString tag = lu.substr(i, j-i);
        i = j-1;
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        s.step_override(alphabet(tag), any_tag, alphabet(tag));
      }
      else
      {
        if(lu[i] == '\\')
        {
          i++;
        }
        s.step_override(lu[i], towlower(lu[i]), any_char, lu[i]);
      }
    }
    s.step(word_boundary, word_boundary_s, word_boundary_ns);
    if(s.isFinal(all_finals))
    {
      last_final = idx+1;
      last_final_out = s.filterFinals(all_finals, alphabet, escaped_chars,
                                      false, 1, INT_MAX,
                                      uppercase, firstupper).substr(1);
    }
    idx++;
  }
  if(last_final == 0)
  {
    write(blank_queue.front(), output);
    blank_queue.pop_front();
    if(!bound_blank_queue.front().empty())
    {
      u_fprintf(output, "[[%S]]", bound_blank_queue.front().c_str());
    }
    bound_blank_queue.pop_front();
    u_fprintf(output, "^%S$", lu_queue.front().c_str());
    lu_queue.pop_front();
    return;
  }

  UString wblank;
  for(size_t i = 0; i < last_final; i++)
  {
    if(!bound_blank_queue[i].empty())
    {
      if(wblank.empty())
      {
        wblank += "[["_u;
      }
      else
      {
        wblank += "; "_u;
      }
      
      wblank += bound_blank_queue[i];
    }
  }
  if(!wblank.empty())
  {
    wblank += "]]"_u;
  }

  size_t output_count = 0;
  size_t pos = 0;
  bool pop_queue = true;
  bool replace_empty = false;
  while(pos != UString::npos && pos != last_final_out.size())
  {
    if (pop_queue) {
      if (output_count < last_final) {
        write(blank_queue[output_count], output);
        if (replace_empty && blank_queue[output_count].empty()) {
          u_fputc(' ', output);
        }
        output_count++;
      } else {
        u_fputc(' ', output);
      }
    }
    write(wblank, output);
    u_fputc('^', output);
    size_t start = pos;
    pos = last_final_out.find("<$"_u, start);
    if(pos == UString::npos)
    {
      write(last_final_out.substr(start), output);
      u_fputc('$', output);
      break;
    }
    else
    {
      write(last_final_out.substr(start, pos-start), output);
      u_fputc('$', output);
      pos += 2;
      if (last_final_out[pos] == '-') {
        pop_queue = false;
        pos++;
      } else if (last_final_out[pos] == '_') {
        pop_queue = true;
        replace_empty = true;
        pos++;
      } else {
        pop_queue = true;
        replace_empty = false;
      }
      pos++;
    }
  }
  for(; output_count < last_final; output_count++)
  {
    if(blank_queue[output_count] != " "_u)
    {
      write(blank_queue[output_count], output);
    }
  }

  blank_queue.erase(blank_queue.begin(), blank_queue.begin()+last_final);
  bound_blank_queue.erase(bound_blank_queue.begin(), bound_blank_queue.begin()+last_final);
  lu_queue.erase(lu_queue.begin(), lu_queue.begin()+last_final);
}

void
LSXProcessor::process(InputFile& input, UFILE* output)
{
  while(true)
  {
    while(!at_end || lu_queue.size() > 0)
    {
      processWord(input, output);
    }
    if(at_null)
    {
      u_fputc('\0', output);
      u_fflush(output);
      at_end = false;
      at_null = false;
    }
    else
    {
      break;
    }
  }
}

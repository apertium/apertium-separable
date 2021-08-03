#include "lsx_processor.h"

#include <lttoolbox/compression.h>
#include <lttoolbox/endian_util.h>
#include <lttoolbox/mmap.h>
#include <cstring>

LSXProcessor::LSXProcessor()
  : alphabet(AlphabetExe(&str_write))
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
  bool mmap = false;
  fpos_t pos;
  if (fgetpos(input, &pos) == 0) {
    char header[4]{};
    if (fread(header, 1, 4, input) == 4 &&
        strncmp(header, HEADER_LTTOOLBOX, 4) == 0) {
      auto features = read_le_64(input);
      if (features >= LTF_UNKNOWN) {
        throw std::runtime_error("FST has features that are unknown to this version of lttoolbox - upgrade!");
      }
      mmap = features & LTF_MMAP;
    }
    else {
      // Old binary format
      fsetpos(input, &pos);
      // of course, lsx-comp would never generate this...
    }
  }

  if (mmap) {
    fgetpos(input, &pos);
    rewind(input);
    mmapping = mmap_file(input, mmap_pointer, mmap_len);
    if (mmapping) {
      void* ptr = mmap_pointer + 12;
      ptr = str_write.init(ptr);

      StringRef let_loc = reinterpret_cast<StringRef*>(ptr)[0];
      vector<int32_t> vec;
      ustring_to_vec32(str_write.get(let_loc), vec);
      alphabetic_chars.insert(vec.begin(), vec.end());
      ptr += sizeof(StringRef);

      ptr = alphabet.init(ptr);

      ptr += sizeof(uint64_t);
      ptr += sizeof(StringRef);
      trans.init(ptr);
    } else {
      fsetpos(input, &pos);

      str_write.read(input);

      uint32_t s = read_le_32(input);
      uint32_t c = read_le_32(input);
      vector<int32_t> vec;
      ustring_to_vec32(str_write.get(s, c), vec);
      alphabetic_chars.insert(vec.begin(), vec.end());

      alphabet.read(input, true);

      read_le_64(input); // number of transducers = 1
      read_le_32(input);
      read_le_32(input); // name
      trans.read(input);
    }
  } else {
    // letters
    int len = Compression::multibyte_read(input);
    while(len > 0) {
      alphabetic_chars.insert(static_cast<UChar32>(Compression::multibyte_read(input)));
      len--;
    }

    // symbols
    fgetpos(input, &pos);
    alphabet.read(input, false);
    fsetpos(input, &pos);
    Alphabet temp;
    temp.read(input);

    len = Compression::multibyte_read(input);
    Compression::string_read(input); // name
    // there should only be 1 transducer in the file
    // so ignore any subsequent ones
    trans.read_compressed(input, temp);
  }
  
  word_boundary = alphabet("<$>"_u);
  any_char = alphabet("<ANY_CHAR>"_u);
  any_tag = alphabet("<ANY_TAG>"_u);
  all_finals.insert(&trans);
  initial_state.init(all_finals);
}

void
LSXProcessor::readNextLU(InputFile& input)
{
  blank_queue.push_back(input.readBlank(false));
  UChar32 c = input.get();
  if (c == U_EOF || (null_flush & c == '\0')) {
    bound_blank_queue.push_back(""_u);
    lu_queue.push_back(""_u);
    at_end = true;
    at_null = (c == '\0');
    return;
  }
  if (c == '[') {
    input.get();
    UString wb = input.finishWBlank();
    bound_blank_queue.push_back(wb.substr(2, wb.size()-4));
    c = input.get();
  } else {
    bound_blank_queue.push_back(""_u);
  }
  if (c != '^') {
    cerr << "invalid stream!\n c = " << c << "\n";
    exit(EXIT_FAILURE);
  }
  UString lu = input.readBlock('^', '$');
  lu_queue.push_back(lu.substr(1, lu.size()-2));
  if(input.eof()) {
    at_end = true;
  }
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
  State s;
  s.init(all_finals);
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
      firstupper = u_isupper(lu[0]);
      uppercase = lu.size() > 1 && u_isupper(lu[1]);
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
        int32_t tag = alphabet.lookupDynamic(lu.substr(i, j-i));
        i = j-1;
        s.step_override(tag, any_tag, tag);
      }
      else
      {
        if(lu[i] == '\\')
        {
          i++;
        }
        s.step_override(lu[i], u_tolower(lu[i]), any_char, lu[i]);
      }
    }
    s.step(word_boundary);
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
  vector<UString> out_lus;
  size_t pos = 0;
  while(pos != UString::npos && pos != last_final_out.size())
  {
    size_t start = pos;
    pos = last_final_out.find("<$>"_u, start);
    if(pos == UString::npos)
    {
      out_lus.push_back(last_final_out.substr(start));
    }
    else
    {
      out_lus.push_back(last_final_out.substr(start, pos-start));
      pos += 3;
    }
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
  
  size_t i = 0;
  for(; i < out_lus.size(); i++)
  {
    if(i < last_final)
    {
      write(blank_queue[i], output);
    }
    else
    {
      u_fputc(' ', output);
    }
    write(wblank, output);
    u_fputc('^', output);
    write(out_lus[i], output);
    u_fputc('$', output);
  }
  for(; i < last_final; i++)
  {
    if(blank_queue[i] != " "_u)
    {
      write(blank_queue[i], output);
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

#include "lsx_processor.h"

#include <lttoolbox/compression.h>

LSXProcessor::LSXProcessor()
{
  escaped_chars.insert(L'[');
  escaped_chars.insert(L']');
  escaped_chars.insert(L'{');
  escaped_chars.insert(L'}');
  escaped_chars.insert(L'^');
  escaped_chars.insert(L'$');
  escaped_chars.insert(L'/');
  escaped_chars.insert(L'\\');
  escaped_chars.insert(L'@');
  escaped_chars.insert(L'<');
  escaped_chars.insert(L'>');

  null_flush = false;
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
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(input)));
    len--;
  }

  // symbols
  alphabet.read(input);
  word_boundary = alphabet(L"<$>");
  any_char = alphabet(L"<ANY_CHAR>");
  any_tag = alphabet(L"<ANY_TAG>");

  len = Compression::multibyte_read(input);
  Compression::wstring_read(input); // name
  // there should only be 1 transducer in the file
  // so ignore any subsequent ones
  trans.read(input, alphabet);

  initial_state.init(trans.getInitial());
  all_finals = trans.getFinals();
}

void
LSXProcessor::readNextLU(FILE* input)
{
  vector<wstring> parts = vector<wstring>(3);
  int loc = 0; // 0 = blank, 1 = bound blank, 2 = LU
  bool box = false; // are we in a [ ] blank
  while(!feof(input))
  {
    wchar_t c = fgetwc_unlocked(input);
    if(null_flush && c == L'\0')
    {
      at_end = true;
      at_null = true;
      break;
    }
    else if(c == L'\\')
    {
      parts[loc] += c;
      c = fgetwc_unlocked(input);
      parts[loc] += c;
    }
    else if(loc == 0 && box)
    {
      if(c == L']')
      {
        box = false;
      }
      parts[loc] += c;
    }
    else if(loc == 0 && c == L'[')
    {
      c = fgetwc_unlocked(input);
      if(c == L'[')
      {
        loc = 1;
      }
      else
      {
        parts[loc] += L'[';
        parts[loc] += c;
        if(c != L']')
        {
          box = true;
        }
        if(c == L'\\')
        {
          parts[loc] += fgetwc_unlocked(input);
        }
      }
    }
    else if(loc == 1 && c == L']')
    {
      c = fgetwc_unlocked(input);
      if(c == L']')
      {
        c = fgetwc_unlocked(input);
        if(c == L'^')
        {
          loc = 2;
        }
        else
        {
          // this situation is invalid
          // but I like making parsers harder to break than required
          // by the standard
          parts[loc] += L"]]";
          parts[loc] += c;
        }
      }
      else
      {
        parts[loc] += L']';
        parts[loc] += c;
        if(c == L'\\')
        {
          parts[loc] += fgetwc_unlocked(input);
        }
      }
    }
    else if(loc == 0 && c == L'^')
    {
      loc = 2;
    }
    else if(loc == 2 && c == L'$')
    {
      break;
    }
    else
    {
      parts[loc] += c;
    }
  }
  if(feof(input))
  {
    at_end = true;
  }
  blank_queue.push_back(parts[0]);
  bound_blank_queue.push_back(parts[1]);
  lu_queue.push_back(parts[2]);
}

void
LSXProcessor::processWord(FILE* input, FILE* output)
{
  if(lu_queue.size() == 0)
  {
    readNextLU(input);
  }
  if(at_end && lu_queue.size() == 1 && lu_queue.back().size() == 0)
  {
    // we're at the final blank, no more work to do
    fputws_unlocked(blank_queue.back().c_str(), output);
    blank_queue.pop_front();
    bound_blank_queue.pop_front();
    lu_queue.pop_front();
    return;
  }
  size_t last_final = 0;
  wstring last_final_out;
  State s;
  s.init(trans.getInitial());
  size_t idx = 0;
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
    wstring lu = lu_queue[idx];
    if(lu.size() == 0)
    {
      break;
    }
    for(size_t i = 0; i < lu.size(); i++)
    {
      if(lu[i] == L'<')
      {
        size_t j = i+1;
        for(; j < lu.size(); j++)
        {
          if(lu[j] == L'\\')
          {
            j++;
          }
          else if(lu[j] == L'>')
          {
            j++;
            break;
          }
        }
        wstring tag = lu.substr(i, j-i);
        i = j-1;
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        s.step_override(alphabet(tag), any_tag, alphabet(tag));
      }
      else
      {
        // TODO: need a version of step_override that can take both
        // upper and lower so we can fix
        // https://github.com/apertium/apertium-separable/issues/21
        int val = towlower(lu[i]);
        if(lu[i] == L'\\')
        {
          i++;
          val = lu[i];
        }
        s.step_override(val, any_char, lu[i]);
      }
    }
    s.step(word_boundary);
    if(s.isFinal(all_finals))
    {
      last_final = idx+1;
      last_final_out = s.filterFinals(all_finals, alphabet, escaped_chars, false, 1, INT_MAX).substr(1);
    }
    idx++;
  }
  if(last_final == 0)
  {
    fputws_unlocked(blank_queue.front().c_str(), output);
    blank_queue.pop_front();
    if(bound_blank_queue.front().size() > 0)
    {
      fputws_unlocked(L"[[", output);
      fputws_unlocked(bound_blank_queue.front().c_str(), output);
      fputws_unlocked(L"]]", output);
    }
    bound_blank_queue.pop_front();
    fputwc_unlocked(L'^', output);
    fputws_unlocked(lu_queue.front().c_str(), output);
    fputwc_unlocked(L'$', output);
    lu_queue.pop_front();
    return;
  }
  vector<wstring> out_lus;
  size_t pos = 0;
  while(pos != wstring::npos && pos != last_final_out.size())
  {
    size_t start = pos;
    pos = last_final_out.find(L"<$>", start);
    if(pos == wstring::npos)
    {
      out_lus.push_back(last_final_out.substr(start));
    }
    else
    {
      out_lus.push_back(last_final_out.substr(start, pos-start));
      pos += 3;
    }
  }
  // TODO: figure out where bound blanks actually go
  size_t i = 0;
  for(; i < out_lus.size(); i++)
  {
    if(i < last_final)
    {
      fputws_unlocked(blank_queue[i].c_str(), output);
      if(bound_blank_queue[i].size() > 0)
      {
        fputws_unlocked(L"[[", output);
        fputws_unlocked(bound_blank_queue[i].c_str(), output);
        fputws_unlocked(L"]]", output);
      }
    }
    else
    {
      fputwc_unlocked(L' ', output);
    }
    fputwc_unlocked(L'^', output);
    fputws_unlocked(out_lus[i].c_str(), output);
    fputwc_unlocked(L'$', output);
  }
  for(; i < last_final; i++)
  {
    if(blank_queue[i] != L" ")
    {
      fputws_unlocked(blank_queue[i].c_str(), output);
    }
  }
  blank_queue.erase(blank_queue.begin(), blank_queue.begin()+last_final);
  bound_blank_queue.erase(bound_blank_queue.begin(), bound_blank_queue.begin()+last_final);
  lu_queue.erase(lu_queue.begin(), lu_queue.begin()+last_final);
}

void
LSXProcessor::process(FILE* input, FILE* output)
{
  while(true)
  {
    while(!at_end || lu_queue.size() > 0)
    {
      processWord(input, output);
    }
    if(at_null)
    {
      fputwc_unlocked(L'\0', output);
      int code = fflush(output);
      if(code != 0)
      {
        wcerr << L"Could not flush output " << errno << endl;
      }
      at_end = false;
      at_null = false;
    }
    else
    {
      break;
    }
  }
}

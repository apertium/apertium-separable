#include "lsx_processor.h"

#include <lttoolbox/file_utils.h>
#include <lttoolbox/string_utils.h>

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
}

void
LSXProcessor::load(FILE *input)
{
  readTransducerSet(input, alphabetic_chars, alphabet, trans);

  // symbols
  word_boundary = alphabet("<$>"_u);
  word_boundary_s = alphabet("<$_>"_u);
  word_boundary_ns = alphabet("<$->"_u);
  any_char = alphabet("<ANY_CHAR>"_u);
  any_tag = alphabet("<ANY_TAG>"_u);
  reading_boundary = alphabet("</>"_u);

  for (auto& it : trans) {
    root.addTransition(0, 0, it.second.getInitial(), 0.0);
    all_finals.insert(it.second.getFinals().begin(),
                      it.second.getFinals().end());
  }
  initial_state.init(&root);
}

void
LSXProcessor::readNextLU(InputFile& input)
{
  UString blank, wblank, lu;
  blank = input.readBlank();
  if (input.peek() == '[') {
    input.get();
    input.get();
    wblank = input.finishWBlank();
    wblank = wblank.substr(2, wblank.size()-4);
  }
  if (input.peek() == '^') {
    input.get();
    lu = input.readBlock('^', '$');
    lu = lu.substr(1, lu.size()-2);
  }

  blank_queue.push_back(blank);
  bound_blank_queue.push_back(wblank);
  lu_queue.push_back(lu);

  if (input.peek() == '\0') {
    input.get();
    at_end = true;
    at_null = true;
  } else if (input.eof()) {
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
    if(lu.empty())
    {
      break;
    }
    State word_initial_state = s;
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
        UString tag = lu.substr(i, j-i);
        i = j-1;
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        s.step_override(alphabet(tag), any_tag, alphabet(tag));
      }
      else if (postgen && lu[i] == '/') {
        s.step(reading_boundary);
        s.merge(word_initial_state);
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
      if (postgen) {
        // TODO: why don't slashes get escaped by filterFinals()?
        last_final_out = StringUtils::substitute(last_final_out, "/"_u, "\\/"_u);
        last_final_out = StringUtils::substitute(last_final_out, "<\\/>"_u, "/"_u);
      }
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

  size_t output_count = 0;
  size_t pos = 0;
  bool pop_queue = true;
  bool replace_empty = false;
  std::vector<UString> blanks;
  std::vector<UString> lus;
  std::vector<int> wblanks_index;
  sorted_vector<int> wblanks_to_merge;
  std::map<UString, int> lu_index;
  for (int i = 0; i < (int)last_final; i++) {
    // note: if a pattern matches 2 identical LUs, we'll lose the first wblank
    lu_index[lu_queue[i]] = i;
    wblanks_to_merge.insert(i);
    if (postgen) {
      auto pieces = StringUtils::split_escaped(lu_queue[i], '/');
      if (!pieces.empty()) lu_index[pieces.back()] = i;
    }
  }
  while(pos != UString::npos && pos != last_final_out.size())
  {
    if (pop_queue) {
      if (output_count < last_final) {
        if (replace_empty && blank_queue[output_count].empty()) {
          blanks.push_back(" "_u);
        } else {
          blanks.push_back(blank_queue[output_count]);
        }
        output_count++;
      } else {
        blanks.push_back(" "_u);
      }
    } else {
      blanks.push_back(""_u);
    }
    size_t start = pos;
    pos = last_final_out.find("<$"_u, start);
    if(pos == UString::npos)
    {
      lus.push_back(last_final_out.substr(start));
      break;
    }
    else
    {
      lus.push_back(last_final_out.substr(start, pos-start));
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
  for (auto& it : lus) {
    auto loc = lu_index.find(it);
    if (loc != lu_index.end()) {
      wblanks_index.push_back(loc->second);
      wblanks_to_merge.erase(loc->second);
    } else {
      wblanks_index.push_back(-1);
    }
  }
  UString trail_blank;
  for(; output_count < last_final; output_count++)
  {
    if(blank_queue[output_count] != " "_u)
    {
      trail_blank += blank_queue[output_count];
    }
  }
  UString merged_wblank;
  for (auto& it : wblanks_to_merge) {
    if (bound_blank_queue[it].empty()) continue;
    if (merged_wblank.empty()) {
      merged_wblank = bound_blank_queue[it];
    } else {
      merged_wblank += "; "_u;
      merged_wblank += bound_blank_queue[it];
    }
  }
  std::vector<UString> wblanks;
  auto changed = wblanks_to_merge.get();
  size_t changed_idx = 0;
  for (size_t i = 0; i < wblanks_index.size(); i++) {
    int src = wblanks_index[i];
    wblanks.push_back((src == -1) ? merged_wblank : bound_blank_queue[src]);
    if (postgen) {
      if (lus[i].find('/') != UString::npos) {
        continue;
      }
      UString tmp;
      if (src != -1) {
        tmp = lu_queue[i];
      }
      else if (changed_idx < changed.size()) {
        auto pieces = StringUtils::split_escaped(lu_queue[changed[changed_idx]], '/');
        tmp += pieces[0];
        tmp += '/';
        tmp += lus[i];
        changed_idx++;
      }
      else {
        tmp += '/';
        tmp += lus[i];
      }
      lus[i].swap(tmp);
    }
  }

  blank_queue.erase(blank_queue.begin(), blank_queue.begin()+last_final);
  bound_blank_queue.erase(bound_blank_queue.begin(), bound_blank_queue.begin()+last_final);
  lu_queue.erase(lu_queue.begin(), lu_queue.begin()+last_final);

  if (repeat_rules) {
    blank_queue.front() = trail_blank + blank_queue.front();
    blank_queue.insert(blank_queue.begin(), blanks.begin(), blanks.end());
    bound_blank_queue.insert(bound_blank_queue.begin(), wblanks.begin(), wblanks.end());
    lu_queue.insert(lu_queue.begin(), lus.begin(), lus.end());
  } else {
    for (size_t i = 0; i < lus.size(); i++) {
      if (wblanks[i].empty()) {
        u_fprintf(output, "%S^%S$", blanks[i].c_str(), lus[i].c_str());
      } else {
        u_fprintf(output, "%S[[%S]]^%S$", blanks[i].c_str(), wblanks[i].c_str(), lus[i].c_str());
      }
    }
    write(trail_blank, output);
  }
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

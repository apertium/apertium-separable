/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <lsx_FSTProcessor.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/exception.h>

#include <iostream>
#include <cerrno>


#ifdef _WIN32
#include <utf8_fwrap.hpp>
#endif

using namespace std;


FSTProcessor::FSTProcessor() :
outOfWord(false),
isLastBlankTM(false)
{
  // escaped_chars chars
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
  escaped_chars.insert(L'+');

  caseSensitive = false;
  dictionaryCase = false;
  do_decomposition = false;
  nullFlush = false;
  nullFlushGeneration = false;
  showControlSymbols = false;
  biltransSurfaceForms = false;
  compoundOnlyLSymbol = 0;
  compoundRSymbol = 0;
  compound_max_elements = 4;

  initial_state = new State();
  current_state = new State();
}

FSTProcessor::~FSTProcessor()
{
  delete current_state;
  delete initial_state;
}

wstring
FSTProcessor::readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc_unlocked(input));
    result += c;
    if(c != L'\\')
    {
      continue;
    }
    else
    {
      result += static_cast<wchar_t>(readEscaped(input));
    }
  }

  if(c != delim2)
  {
    streamError();
  }

  return result;
}

int
FSTProcessor::readPostgeneration(FILE *input)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  wchar_t val = static_cast<wchar_t>(fgetwc_unlocked(input));
  int altval = 0;
  if(feof(input))
  {
    return 0;
  }

  switch(val)
  {
    case L'<':
      altval = static_cast<int>(alphabet(readFullBlock(input, L'<', L'>')));
      input_buffer.add(altval);
      return altval;

    case L'[':
      blankqueue.push(readFullBlock(input, L'[', L']'));
      input_buffer.add(static_cast<int>(L' '));
      return static_cast<int>(L' ');

    case L'\\':
      val = static_cast<wchar_t>(fgetwc_unlocked(input));
      if(escaped_chars.find(val) == escaped_chars.end())
      {
        streamError();
      }
      input_buffer.add(static_cast<int>(val));
      return val;

    default:
      input_buffer.add(val);
      return val;
  }
}

int
FSTProcessor::readGeneration(FILE *input, FILE *output)
{
  wint_t val = fgetwc_unlocked(input);

  if(feof(input))
  {
    return 0x7fffffff;
  }

  if(outOfWord)
  {
    if(val == L'^')
    {
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else if(val == L'\\')
    {
      fputwc_unlocked(val, output);
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
      fputwc_unlocked(val,output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    else
    {
      fputwc_unlocked(val, output);
      skipUntil(input, output, L'^');
      val = fgetwc_unlocked(input);
      if(feof(input))
      {
        return 0x7fffffff;
      }
    }
    outOfWord = false;
  }

  if(val == L'\\')
  {
    val = fgetwc_unlocked(input);
    return static_cast<int>(val);
  }
  else if(val == L'$')
  {
    outOfWord = true;
    return static_cast<int>(L'$');
  }
  else if(val == L'<')
  {
    wstring cad = L"";
    cad += static_cast<wchar_t>(val);
    while((val = fgetwc_unlocked(input)) != L'>')
    {
      if(feof(input))
      {
	streamError();
      }
      cad += static_cast<wchar_t>(val);
    }
    cad += static_cast<wchar_t>(val);

    return alphabet(cad);
  }
  else if(val == L'[')
  {
    fputws_unlocked(readFullBlock(input, L'[', L']').c_str(), output);
    return readGeneration(input, output);
  }
  else
  {
    return static_cast<int>(val);
  }

  return 0x7fffffff;
}

void
FSTProcessor::load(FILE *input)
{
  // letters
  int len = Compression::multibyte_read(input);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(input)));
    len--;
  }

  // symbols
  alphabet.read(input);

  len = Compression::multibyte_read(input);

  while(len > 0)
  {
    int len2 = Compression::multibyte_read(input);
    wstring name = L"";
    while(len2 > 0)
    {
      name += static_cast<wchar_t>(Compression::multibyte_read(input));
      len2--;
    }
    transducers[name].read(input, alphabet);
    len--;
  }

}

void
FSTProcessor::initGeneration()
{
  for(map<wstring, TransExe, Ltstr>::iterator it = transducers.begin(),
                                             limit = transducers.end();
      it != limit; it++)
  {
    all_finals.insert(it->second.getFinals().begin(),
                      it->second.getFinals().end());
  }
}

void
FSTProcessor::lsx(FILE *input, FILE *output)
{
  vector<State> new_states, alive_states;
  wstring blank, out, in, alt_out, alt_in;
  bool outOfWord = true;
  bool finalFound = false;
  bool plus_thing = false;

  alive_states.push_back(*initial_state);

  while(!feof(input))
  {
    int val = fgetwc(input);

    if(val == L'+' && isEscaped(val) && !outOfWord)
    {
      val = L'$';
      plus_thing = true;
    }

    if((val == L'^' && isEscaped(val) && outOfWord) || feof(input))
    {
      blankqueue.push(blank);

      if(alive_states.size() == 0)
      {
        if(blankqueue.size() > 0)
        {
          fputws(blankqueue.front().c_str(), output);
          fflush(output);
          blankqueue.pop();
        }

        alive_states.push_back(*initial_state);

        alt_in = L"";
        for(int i=0; i < (int) in.size(); i++) // FIXME indexing
        {
          alt_in += in[i];
          if(in[i] == L'$' && in[i+1] == L'^' && blankqueue.size() > 0)
          {
            // in.insert(i+1, blankqueue.front().c_str());
            alt_in += blankqueue.front().c_str();
            blankqueue.pop();
          }
        }
        in = alt_in;
        fputws(in.c_str(), output);
        fflush(output);
        in = L"";
        finalFound = false;
      }
      else if(finalFound && alive_states.size() == 1)
      {
        finalFound = false;
      }

      blank = L"";
      in += val;
      outOfWord = false;
      continue;
    }

//    wcerr << L"\n[!] " << (wchar_t)val << L" ||| " << outOfWord << endl;

    if(outOfWord)
    {
      blank += val;
      continue;
    }

    if((feof(input) || val == L'$') && !outOfWord) // && isEscaped(val)
    {
      new_states.clear();
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        State s = *it;
//        wcerr << endl << L"[0] FEOF | $ | " << s.size() << L" | " << s.isFinal(all_finals) << endl;
        s.step(alphabet(L"<$>"));
//        wcerr << endl << L"[1] FEOF | $ | " << s.size() << L" | " << s.isFinal(all_finals) << endl;
        if(s.size() > 0)
        {
          new_states.push_back(s);
        }

/*        if(s.isFinal(all_finals))
        {
          out += s.filterFinals(all_finals, alphabet, escaped_chars);
          new_states.push_back(*initial_state);
        }*/

        if(s.isFinal(all_finals))
        {
          new_states.clear();
          out = s.filterFinals(all_finals, alphabet, escaped_chars);

          new_states.push_back(*initial_state);

          alt_out = L"";
          for (int i=0; i < (int) out.size(); i++)
          {
            wchar_t c = out.at(i);
            if(c == L'/')
            {
              alt_out += L'^';
            }
            else if(out[i-1] == L'<' && c == L'$' && out[i+1] == L'>') // indexing
            {
              alt_out += c;
              alt_out += L'^';
            }
            else if(!(c == L'<' && out[i+1] == L'$' && out[i+2] == L'>') && !(out[i-2] == L'<' && out[i-1] == L'$' && c == L'>'))
            {
              alt_out += c;
            }
          }
          out = alt_out;


          if(out[out.length()-1] == L'^')
          {
            out = out.substr(0, out.length()-1); // extra ^ at the end
            if(plus_thing)
            {
              out[out.size()-1] = L'+';
              plus_thing = false;
            }
          }
          else // take# out ... of
          {
            for(int i=out.length()-1; i>=0; i--) // indexing
            {
              if(out.at(i) == L'$')
              {
                out.insert(i+1, L" ");
                break;
              }
            }
            out += L'$';
          }

          if(blankqueue.size() > 0)
          {
            fputws(blankqueue.front().c_str(), output);
            blankqueue.pop();
          }

          alt_out = L"";
          for(int i=0; i < (int) out.size(); i++) // indexing
          {
            if((out.at(i) == L'$') && blankqueue.size() > 0)
            {
              alt_out += out.at(i);
              alt_out += blankqueue.front().c_str();
              blankqueue.pop();
            }
            else if((out.at(i) == L'$') && blankqueue.size() == 0 && i != (int) out.size()-1)
            {
              alt_out += out.at(i);
              alt_out += L' ';
            }
            else if(out.at(i) == L' ' && blankqueue.size() > 0)
            {
              alt_out += blankqueue.front().c_str();
              blankqueue.pop();
            }
            else
            {
              alt_out += out.at(i);
            }
          }
          out = alt_out;

          fputws(out.c_str(), output);
          flushBlanks(output);
          finalFound = true;
          out = L"";
          in = L"";
        }
      }

      alive_states.swap(new_states);
      outOfWord = true;

      if(!finalFound)
      {
        in += val; //do not remove
      }
      continue;
    }

    if(!outOfWord) // && (!(feof(input) || val == L'$')))
    {
      if(val == L'<') // tag
      {
        wstring tag = readFullBlock(input, L'<', L'>');
        in += tag;
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        val = static_cast<int>(alphabet(tag));
      }
      else
      {
        in += (wchar_t) val;
      }

      new_states.clear();
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        State s = *it;
        if(val < 0)
        {
          s.step_override(val, alphabet(L"<ANY_TAG>"), val);
        }
        else if(val > 0)
        {
          int val_lowercase = towlower(val);
          s.step_override(val_lowercase, alphabet(L"<ANY_CHAR>"), val); // FIXME deal with cases! in step_override
        }

        if(s.size() > 0)
        {
          new_states.push_back(s);
        }

        // if(s.isFinal(all_finals)) /* ADDITION */
        // {
        //
        // .......... same as above
        //
        // }

      }
      alive_states.swap(new_states);
    }
  }

  flushBlanks(output);
}

void
FSTProcessor::generation(FILE *input, FILE *output, GenerationMode mode)
{
  if(getNullFlush())
  {
    generation_wrapper_null_flush(input, output, mode);
  }

  State current_state = *initial_state;
  wstring sf = L"";

  outOfWord = false;

  skipUntil(input, output, L'^');
  int val;

  while((val = readGeneration(input, output)) != 0x7fffffff)
  {
    if(sf == L"" && val == L'=')
    {
      fputwc(L'=', output);
      val = readGeneration(input, output);
    }

    if(val == L'$' && outOfWord)
    {
      if(sf[0] == L'*' || sf[0] == L'%')
      {
	if(mode != gm_clean && mode != gm_tagged_nm)
        {
	  writeEscaped(sf, output);
	}
	else if (mode == gm_clean)
	{
	  writeEscaped(sf.substr(1), output);
	}
	else if(mode == gm_tagged_nm)
	{
	  fputwc_unlocked(L'^', output);
	  writeEscaped(removeTags(sf.substr(1)), output);
	  fputwc_unlocked(L'/', output);
          writeEscapedWithTags(sf, output);
	  fputwc_unlocked(L'$', output);
	}
      }
      else if(sf[0] == L'@')
      {
        if(mode == gm_all)
        {
          writeEscaped(sf, output);
        }
        else if(mode == gm_clean)
        {
          writeEscaped(removeTags(sf.substr(1)), output);
        }
        else if(mode == gm_unknown)
        {
          writeEscaped(removeTags(sf), output);
        }
        else if(mode == gm_tagged)
        {
          writeEscaped(removeTags(sf), output);
        }
        else if(mode == gm_tagged_nm)
        {
	  fputwc_unlocked(L'^', output);
	  writeEscaped(removeTags(sf.substr(1)), output);
	  fputwc_unlocked(L'/', output);
          writeEscapedWithTags(sf, output);
	  fputwc_unlocked(L'$', output);
        }
      }
      else if(current_state.isFinal(all_finals))
      {
        bool firstupper = false, uppercase = false;
        if(!dictionaryCase)
        {
          uppercase = sf.size() > 1 && iswupper(sf[1]);
          firstupper= iswupper(sf[0]);
        }

        if(mode == gm_tagged || mode == gm_tagged_nm)
        {
	  fputwc_unlocked(L'^', output);
        }

        fputws_unlocked(current_state.filterFinals(all_finals, alphabet,
                                                  escaped_chars,
                                                  uppercase, firstupper).substr(1).c_str(),
						  output);
        if(mode == gm_tagged || mode == gm_tagged_nm)
        {
	  fputwc_unlocked(L'/', output);
	  writeEscapedWithTags(sf, output);
	  fputwc_unlocked(L'$', output);
        }

      }
      else
      {
        if(mode == gm_all)
        {
	  fputwc_unlocked(L'#', output);
	  writeEscaped(sf, output);
        }
        else if(mode == gm_clean)
        {
          writeEscaped(removeTags(sf), output);
        }
        else if(mode == gm_unknown)
        {
          if(sf != L"")
          {
            fputwc_unlocked(L'#', output);
            writeEscaped(removeTags(sf), output);
          }
        }
        else if(mode == gm_tagged)
        {
          fputwc_unlocked(L'#', output);
          writeEscaped(removeTags(sf), output);
        }
        else if(mode == gm_tagged_nm)
        {
	  fputwc_unlocked(L'^', output);
	  writeEscaped(removeTags(sf), output);
	  fputwc_unlocked(L'/', output);
	  fputwc_unlocked(L'#', output);
          writeEscapedWithTags(sf, output);
	  fputwc_unlocked(L'$', output);
        }
      }

      current_state = *initial_state;
      sf = L"";
    }
    else if(iswspace(val) && sf.size() == 0)
    {
      // do nothing
    }
    else if(sf.size() > 0 && (sf[0] == L'*' || sf[0] == L'%' ))
    {
      alphabet.getSymbol(sf, val);
    }
    else
    {
      alphabet.getSymbol(sf,val);
      if(current_state.size() > 0)
      {
	if(!alphabet.isTag(val) && iswupper(val) && !caseSensitive)
	{
          if(mode == gm_carefulcase)
          {
	    current_state.step_careful(val, towlower(val));
          }
          else
          {
	    current_state.step(val, towlower(val));
          }
	}
	else
	{
	  current_state.step(val);
	}
      }
    }
  }
}

void
FSTProcessor::postgeneration(FILE *input, FILE *output)
{
  if(getNullFlush())
  {
    postgeneration_wrapper_null_flush(input, output);
  }

  bool skip_mode = true;
  State current_state = *initial_state;
  wstring lf = L"";
  wstring sf = L"";
  int last = 0;
  set<wchar_t> empty_escaped_chars;

  while(wchar_t val = readPostgeneration(input))
  {
    if(val == L'~')
    {
      skip_mode = false;
    }

    if(skip_mode)
    {
      if(iswspace(val))
      {
	printSpace(val, output);
      }
      else
      {
        if(isEscaped(val))
        {
          fputwc_unlocked(L'\\', output);
        }
      	fputwc_unlocked(val, output);
      }
    }
    else
    {
      // test for final states
      if(current_state.isFinal(all_finals))
      {
        bool firstupper = iswupper(sf[1]);
        bool uppercase = sf.size() > 1 && firstupper && iswupper(sf[2]);
        lf = current_state.filterFinals(all_finals, alphabet,
					empty_escaped_chars,
					uppercase, firstupper, 0);

        // case of the beggining of the next word

        wstring mybuf = L"";
        for(size_t i = sf.size(); i > 0; --i)
        {
          if(!isalpha(sf[i-1]))
          {
            break;
          }
          else
          {
            mybuf = sf[i-1] + mybuf;
          }
        }

        if(mybuf.size() > 0)
        {
          bool myfirstupper = iswupper(mybuf[0]);
          bool myuppercase = mybuf.size() > 1 && iswupper(mybuf[1]);

          for(size_t i = lf.size(); i > 0; --i)
          {
            if(!isalpha(lf[i-1]))
            {
              if(myfirstupper && i != lf.size())
              {
                lf[i] = towupper(lf[i]);
              }
              else
              {
                lf[i] = towlower(lf[i]);
              }
              break;
            }
            else
            {
              if(myuppercase)
              {
                lf[i-1] = towupper(lf[i-1]);
              }
              else
              {
                lf[i-1] = towlower(lf[i-1]);
              }
            }
          }
        }

        last = input_buffer.getPos();
      }

      if(!iswupper(val) || caseSensitive)
      {
        current_state.step(val);
      }
      else
      {
        current_state.step(val, towlower(val));
      }

      if(current_state.size() != 0)
      {
        alphabet.getSymbol(sf, val);
      }
      else
      {
        if(lf == L"")
	{
          unsigned int mark = sf.size();
          for(unsigned int i = 1, limit = sf.size(); i < limit; i++)
          {
            if(sf[i] == L'~')
            {
              mark = i;
              break;
            }
          }
	  fputws_unlocked(sf.substr(1, mark-1).c_str(), output);
          if(mark == sf.size())
          {
	    input_buffer.back(1);
          }
          else
          {
            input_buffer.back(sf.size()-mark);
	  }
	}
	else
	{
	  fputws_unlocked(lf.substr(1,lf.size()-3).c_str(), output);
	  input_buffer.setPos(last);
          input_buffer.back(2);
          val = lf[lf.size()-2];
	  if(iswspace(val))
	  {
	    printSpace(val, output);
	  }
	  else
	  {
	    if(isEscaped(val))
	    {
	      fputwc_unlocked(L'\\', output);
	    }
	    fputwc_unlocked(val, output);
	  }
	}

	current_state = *initial_state;
	lf = L"";
	sf = L"";
	skip_mode = true;
      }
    }
  }

  // print remaining blanks
  flushBlanks(output);
}


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

  initial_state = {};
  current_state = new State();
}

FSTProcessor::~FSTProcessor()
{
  delete current_state;
}

void
FSTProcessor::streamError()
{
  throw Exception("Error: Malformed input stream.");
}

wchar_t
FSTProcessor::readEscaped(FILE *input)
{
  if(feof(input))
  {
    streamError();
  }

wchar_t val = static_cast<wchar_t>(fgetwc_unlocked(input));

  if(feof(input) || escaped_chars.find(val) == escaped_chars.end())
  {
    streamError();
  }

  return val;
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

bool
FSTProcessor::isEscaped(wchar_t const c) const
{
  return escaped_chars.find(c) != escaped_chars.end();
}

void
FSTProcessor::calcInitial()
{
  for(map<wstring, TransExe, Ltstr>::iterator it = transducers.begin(),
                                             limit = transducers.end();
      it != limit; it++)
  {
    root.addTransition(0, 0, it->second.getInitial());
  }

  initial_state.init(&root);
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
FSTProcessor::flushBlanks(FILE *output)
{
  for(unsigned int i = blankqueue.size(); i > 0; i--)
  {
    fputws_unlocked(blankqueue.front().c_str(), output);
    blankqueue.pop();
  }
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
FSTProcessor::lsx(FILE *input, FILE *output)
{
  vector<State> new_states, alive_states;
  wstring blank, out, in, alt_out, alt_in;
  bool outOfWord = true;
  bool finalFound = false;
  bool plus_thing = false;

  alive_states.push_back(initial_state);

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

        alive_states.push_back(initial_state);

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

          new_states.push_back(initial_state);

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

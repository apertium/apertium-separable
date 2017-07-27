#include <lttoolbox/lt_locale.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/state.h>

/* get the text between delim1 and delim2 */
wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);

wstring
readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc(input));
    result += c;
  }

  return result;
}

int main (int argc, char** argv)
{
  if(argc != 2)
  {
    wcout << L"./lsx-comp <bin file>" << endl;
    exit(0);
  }

  Alphabet alphabet;
  TransExe transducer;

  LtLocale::tryToSetLocale();

  FILE *fst = fopen(argv[1], "r");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }

  set<wchar_t> alphabetic_chars;
  int len = Compression::multibyte_read(fst);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(fst)));
    len--;
  }

  alphabet.read(fst);

  len = Compression::multibyte_read(fst);
  len = Compression::multibyte_read(fst);

  wstring name = L"";
  while(len > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--;
  }

  transducer.read(fst, alphabet);

  set<Node *> anfinals;
  set<wchar_t> escaped_chars;

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

  State *initial_state = new State();
  initial_state->init(transducer.getInitial());
  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

  vector<State> new_states;
  vector<State> alive_states;
  list<wstring> blanks;

  alive_states.push_back(*initial_state);

  FILE *input = stdin;
  FILE *output = stdout;

  bool outOfWord = true;
  bool isEscaped = false;
  bool finalFound = false;
  bool leading = false;

  wstring in = L"";
  wstring out;

  while(!feof(input))
  {
     int val = fgetwc(input);
    if(alive_states.size() == 0 && !finalFound)
    {
      alive_states.push_back(*initial_state);
      fputws(in.c_str(), output);
      in = L"";
      leading = true;
    }
    else if(alive_states.size() == 0 && finalFound)
    {
      in = L"";
      finalFound = false;
    }

    if((val == L'^' && !isEscaped && outOfWord))
    {
      outOfWord = false;
      in += val;
      continue;
    }

    if((feof(input) || val == L'$') && !isEscaped && !outOfWord)
    {
      new_states.clear();
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        State s = *it;
        s.step(alphabet(L"<$>"));
        if(s.size() > 0)
        {
          new_states.push_back(s);
        }

        if(s.isFinal(anfinals))
        {
          out += s.filterFinals(anfinals, alphabet, escaped_chars);
          new_states.push_back(*initial_state);
        }
      }

      alive_states.swap(new_states);
      outOfWord = true;
      in += val;
      continue;
    }

    if(!outOfWord)
    {
      if(val == L'<') // tag
      {
        wstring tag = readFullBlock(input, L'<', L'>');
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        val = static_cast<int>(alphabet(tag));
        in += tag;
      }
      else
      {
        in += (wchar_t) val;
      }

      new_states.clear();
      wstring res = L"";
      for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
      {
        res = L"";
        State s = *it;
        if(val < 0)
        {
          s.step_override(val, alphabet(L"<ANY_TAG>"), val);
        }
        else if(val > 0)
        {
          s.step_override(val, alphabet(L"<ANY_CHAR>"), val); // FIXME deal with cases!
        }

        if(s.size() > 0)
        {
          new_states.push_back(s);
        }

        if(s.isFinal(anfinals))
        {
          out = s.filterFinals(anfinals, alphabet, escaped_chars);
          new_states.push_back(*initial_state);

          finalFound = true;

          for (int i=0; i < (int) out.size(); i++)
          {
            wchar_t c = out[i];
            /* FIXME these hacks */
            // if(c == L'/')
            // {
            //   out[i] = L'^';
            // }
            // else if(c == L'$')
            // {
            //   out[i-1] = L'$';
            //   out[i] = L' ';
            //   out[i+1] = L'^';
            // }
          }
          out = out.substr(0, out.length()-3); // remove extra trailing '$ ^' : '^ ' is excess, '$' will be added in the next loop with fputws(in,output)
          /* FIXME another hack */
          // if(leading) {
          //   fputwc(L' ', output);
          // }
          fputws(out.c_str(), output);
        }
      }
      alive_states.swap(new_states);
    }
    else if(outOfWord) // FIXME need to deal with superblank stuff
    {
      // wcout << (wchar_t) val << endl;
      // wstring blank = L"";

      // while(val != L'^' || feof(input))
      // {
      //   blank += static_cast<wchar_t>(val);
      //   val = fgetwc(input);
      // }
      // skip = true;

      // if(val == L' ')
      // {
      //   wstring blank = L"";
      //   blank += static_cast<wchar_t>(val);
      //   blanks.push_back(blank);
      //   // wcout << "b" << blank << "b";
      // }
      // else if(val == L'[') // tag
      // {
      //   wstring blank = readFullBlock(input, L'[', L']');
      //   blanks.push_back(blank);
      //   wcout << "b"<< blank<<"B";
      // }
      // FIXME anything between $ and ^
      // else
      // {
        fputwc(val, output);
        continue;
      // }

      if(blanks.size() > 0)
      {
        // wcout << blanks.front();
        blanks.pop_front();
      }
    }
    else
    {
      wcerr << L"outOfWord error" << endl;
    }
  }

  return 0;
}
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/state.h>
#include <lttoolbox/fst_processor.h>

wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);
// wchar_t readEscaped(FILE *input);
// void streamError();
void flushBlanks(FILE *output);

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
    if(c != L'\\')
    {
      continue;
    }
    // else
    // {
    //   result += static_cast<wchar_t>(readEscaped(input));
    // }
  }
  // if(c != delim2)
  // {
  //   streamError();
  // }
  return result;
}

// void
// flushBlanks(FILE *output)
// {
//   for(unsigned int i = blankqueue.size(); i > 0; i--)
//   {
//     fputws_unlocked(blankqueue.front().c_str(), output);
//     blankqueue.pop();
//   }
// }

int main (int argc, char** argv)
{
  if(argc != 2)
  {
    wcout << L"./lsx-comp <bin file>" << endl;
    exit(0);
  }

  Alphabet alphabet;
  TransExe transducer;
  FSTProcessor fstp;

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
  list<wstring> blankqueue;

  alive_states.push_back(*initial_state);

  FILE *input = stdin;
  FILE *output = stdout;

  bool outOfWord = true;
  bool isEscaped = false;
  bool finalFound = false;

  wstring in = L"";
  wstring out;

  while(!feof(input))
  {
    int val = fgetwc(input);

    if(outOfWord)
    {
      // wstring blank = L"";
      // while(val != L'^' && !feof(input))
      // {
      //   blank += val;
      //   val = fgetwc(input);
      // }
      // blankqueue.push_back(blank);
      // fputwc(val, output);
      // fputws(blankqueue.front().c_str(),output);
      // fflush(output);
      // blankqueue.pop_front();
      // outOfWord = false;
      // continue;
      // fputwc(val,output);
    }
    if(val == L'^' && !isEscaped && outOfWord)
    {
      outOfWord = false;
      in += val;
      continue;
    }

    if(alive_states.size() == 0 && !finalFound)
    {
      alive_states.push_back(*initial_state);
      fputws(in.c_str(), output);
      fflush(output);
      in = L"";
    }
    else if(alive_states.size() == 0 && finalFound)
    {
      in = L"";
      finalFound = false;
    }
    if((feof(input) || val == L'$') && !isEscaped && !outOfWord)
    {
      // wcout <<L"($$$)";
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
            // wchar_t c = out[i];
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
          // out = out.substr(1, out.length()-1);
          /* FIXME another hack */
          fputws(out.c_str(), output);
          fflush(output);
        }
      }
      alive_states.swap(new_states);
    }
    else if(outOfWord) // FIXME need to deal with superblnk stuff
    {
        fputwc(val, output);

         // fputws(blankqueue.front().c_str(),output);
         // fflush(output);
         // blankqueue.pop_front();
         // continue;


    //   if(blankqueue.size() > 0)
    //   {
    //     fputws(blankqueue.front().c_str(), output);
    //     blankqueue.pop_front();
    //   }
    }
    // else
    // {
    //   wcerr << L"outOfWord error" << endl;
    // }
  }
  return 0;
}
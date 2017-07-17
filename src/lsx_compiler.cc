#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <stdlib.h>

#include <lttoolbox/compiler.h>
#include <lttoolbox/xml_parse_util.h>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

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
  if(argc != 3)
  {
    wcout << L"lsx-comp <dix file> <bin file>" << endl;
    exit(0);
  }

  /* compile */

  Compiler c;
  c.parse(argv[1], L"lr");

  FILE* fst = fopen(argv[2], "w+");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }
  c.write(fst);
  fclose(fst);

  /* process */

  Alphabet alphabet;
  TransExe transducer;

  LtLocale::tryToSetLocale();

  fst = fopen(argv[2], "r");

  set<wchar_t> alphabetic_chars;
  int len = Compression::multibyte_read(fst);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(fst)));
    len--;
  }

  alphabet.read(fst);
  wcerr << L"alphabet_size: " << alphabet.size() << endl;

  len = Compression::multibyte_read(fst);
  len = Compression::multibyte_read(fst);

  wcerr << len << endl;
  wstring name = L"";
  while(len > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--;
  }
  wcerr << name << endl;

  transducer.read(fst, alphabet);

  FILE *input = stdin;
  FILE *output = stdout;

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

  State *initial_state;
  initial_state = new State();
  initial_state->init(transducer.getInitial());
  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());


  vector<State> new_states;
  vector<State> alive_states;

  alive_states.push_back(*initial_state);

  bool outOfWord = true;
  bool isEscaped = false;

  while(!feof(input))
  {
      int val = fgetwc(input); // read 1 wide char

      wcerr << L"| " << (wchar_t)val << L" | val: " << val << L"  || s.size(): " << alive_states.size() << L" || " << outOfWord << endl;

      if(/*val == L'^' && */ !isEscaped && outOfWord)
      {
        outOfWord = false;
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
            wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
            wcerr << "FINAL: " << out << endl;
            new_states.push_back(*initial_state);
          }
        }
        alive_states.swap(new_states);

        outOfWord = true;
        continue;
      }

      if(val == L'<' && !outOfWord) // if in tag, get the whole tag and modify if necessary
      {
        wstring tag = L"";
        tag = readFullBlock(input, L'<', L'>');
        if(!alphabet.isSymbolDefined(tag))
        {
          alphabet.includeSymbol(tag);
        }
        val = static_cast<int>(alphabet(tag));

        fwprintf(stderr, L"tag %S: %d\n", tag.c_str(), val);
      }

      if(!outOfWord)
      {
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
            s.step_override(val, alphabet(L"<ANY_CHAR>"), val); // deal with cases!
          }
          if(s.size() > 0)
          {
            new_states.push_back(s);
          }
          wcerr << L"|   | " << (wchar_t) val << L" " << L"size: " << s.size() << L" final: " << s.isFinal(anfinals) << endl;
          wcerr << L"|   | cur: " << s.getReadableString(alphabet) << endl;
        }
        alive_states.swap(new_states);
      }

      if(outOfWord)
      {
        continue;
      }

  }

  return 0;
}

#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <stdlib.h>

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
  wchar_t c = delim1;
  result += c;

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
    wcout << L"./lsx-proc <bin file>" << endl;
    exit(0);
  }

  Alphabet alphabet;
  TransExe transducer;
  LtLocale::tryToSetLocale();

  FILE* fst = fopen(argv[1], "r");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << argv[2] << "'." << endl;
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
  wcerr << L"alphabet_size: " << alphabet.size() << endl; //NOTE

  len = Compression::multibyte_read(fst);
  len = Compression::multibyte_read(fst);

  wstring name = L"";
  while(len > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--;
  }
  wcerr << name << endl; //NOTE

  transducer.read(fst, alphabet);

  FILE *input = stdin;
  FILE *output = stdout;

  set<Node *> anfinals;
  vector<State> new_states, alive_states;
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
  alive_states.push_back(*initial_state);

  bool outOfWord = true;
  bool isEscaped = false;

  int tagCount = 0;

  int val = 0;
  while(!feof(input))
  {
    val = fgetwc(input); // read 1 wide char

    if(val == L'<') // tag
    {
      wstring tag = L"";
      tag = readFullBlock(input, L'<', L'>');
      if(!alphabet.isSymbolDefined(tag))
      {
        alphabet.includeSymbol(tag);
      }
      val = static_cast<int>(alphabet(tag));
      tagCount++;
    }
    new_states.clear();

      if(val == L'^') {
        outOfWord = false;
        continue;
      }

    wcout << "val: " << val << " " << (char) val << " alive_states size: " << alive_states.size() << " tagCount: " << tagCount << " isTag: " << alphabet.isTag(val) << endl;
    for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
    {
      State s = *it;

      if(val == L'$') {
        s.step(alphabet(L"<$>"));
        wcout << " wb" << endl;
        tagCount = 0;
      }
      else if(alphabet.isTag(val) && tagCount <= 1) {
        wcout << " first tag" << endl;
        // if ( alphabet(L"<vblex>") == val) { wcout << "equal" ;} else { wcout << "not equal" ;}
        // wcout << "vblex defined? " << alphabet.isSymbolDefined(L"<vblex>") << endl;
        //s.step(val);
        s.step_override(val, alphabet(L"<ANY_TAG>"), val);
        // s.step(-18);
        // s.step(alphabet(L"<vblex>"));
        // s.step_override(val, alphabet(L"<vblex>"), val);

      } else if(/*alphabet.isTag(val) &&*/ tagCount > 1) {
        wcout << " second tag" << endl;
        s.step_override(val, alphabet(L"<ANY_TAG>"), val);
      }
      else if(val > 0)
      {
        s.step_override(val, alphabet(L"<ANY_CHAR>"), val);
        //s.step(val);
        wcout << " original char: " << val << endl;
      }
      else {
        wcout << "error?" << endl;
      }
      if(s.size() > 0) // alive if the vector isn't empty
      {
        wcout << "pushing new states" << endl;
        new_states.push_back(s);
      }

      if(s.isFinal(anfinals))
      {
        wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
        wcerr << "FINAL: " << out << endl;
        new_states.push_back(*initial_state);
      }
    }
    // wcout << "new-states size: " << new_states.size() << endl;
    alive_states.swap(new_states);


  //
  //   if(val == L'$' && !isEscaped /*&& outOfWord*/)
  //   {
  //     wcout << "val: " << val << " " << (char) val << endl;
  //     outOfWord = false;
  //     continue;
  //   }
  //
  //   if((feof(input) || val == L'$') && !isEscaped /*&& !outOfWord*/)
  //   {
  //     wcout << "val: " << val << " " << (char) val << endl;
  //     new_states.clear();
  //     for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
  //     {
  //       State s = *it;
  //       s.step(alphabet(L"<$>"));
  //       wcout << "alive_states size: " << alive_states.size() << endl;
  //       if(s.size() > 0)
  //       {
  //         new_states.push_back(s);
  //       }
  //
  //       if(s.isFinal(anfinals))
  //       {
  //         wstring out = s.filterFinals(anfinals, alphabet, escaped_chars);
  //         wcerr << "FINAL: " << out << endl;
  //         new_states.push_back(*initial_state);
  //       }
  //     }
  //     alive_states.swap(new_states);
  //
  //     outOfWord = true;
  //     continue;
  //   }
  //
  //   if(val == L'<' /*&& !outOfWord*/) // tag
  //   {
  //     wstring tag = L"";
  //     tag = readFullBlock(input, L'<', L'>');
  //     if(!alphabet.isSymbolDefined(tag))
  //     {
  //       alphabet.includeSymbol(tag);
  //     }
  //     val = static_cast<int>(alphabet(tag));
  //     wcout << "val: " << val << " " << (char) val << endl;
  //
  //     // fwprintf(stderr, L"tag %S: %d\n", tag.c_str(), val);
  //   }
  //
  //   if(!outOfWord)
  //   {
  //     wcout << "val: " << val << " " << (char) val << endl;
  //     new_states.clear();
  //     wstring res = L"";
  //     for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
  //     {
  //       res = L"";
  //       State s = *it;
  //       if(val < 0)
  //       {
  //         s.step_override(val, alphabet(L"<ANY_TAG>"), val);
  //       }
  //       else if(val > 0)
  //       {
  //         s.step_override(val, alphabet(L"<ANY_CHAR>"), val); // deal with cases!
  //       }
  //       if(s.size() > 0)
  //       {
  //         new_states.push_back(s);
  //       }
  //       // wcerr << L"|   | " << (wchar_t) val << L" " << L"size: " << s.size() << L" final: " << s.isFinal(anfinals) << endl;
  //       // wcerr << L"|   | cur: " << s.getReadableString(alphabet) << endl;
  //
  //     }
  //     alive_states.swap(new_states);
  //   }
  //
  //   if(outOfWord)
  //   {
  //     continue;
  //   }
  }

  return 0;
}

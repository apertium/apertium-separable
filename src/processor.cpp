#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
// #include <fstream>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

int main (int argc, char** argv) 
{
  Alphabet alphabet;
  TransExe transducer;

  LtLocale::tryToSetLocale();


  FILE *fst = fopen(argv[1], "r");
  
  set<wchar_t> alphabetic_chars;
  int len = Compression::multibyte_read(fst);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(fst)));
    len--;
  }

  alphabet.read(fst);
  wcout << L"alphabet_size: " << alphabet.size() << endl;

  len = Compression::multibyte_read(fst);
  
  len = Compression::multibyte_read(fst);
  wcout << len << endl;
  wstring name = L"";
  while(len > 0) 
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--; 
  }
  wcout << name << endl;


  transducer.read(fst, alphabet);

  FILE *input = stdin;
  FILE *output = stdout;

  vector<State> alive_states;
  set<Node *> anfinals;
  set<wchar_t> escaped_chars;
  State *initial_state;

  initial_state->init(transducer.getInitial());
  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

  /*
  processing
  */

  int line_number = 0;
  bool accepted = true;

  while(!feof(input)) 
  {
      int val = fgetwc_unlocked(input);

      wcout << (wchar_t)val << endl;
  }

  return 0;
}

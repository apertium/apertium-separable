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

  alphabet.read(fst);
  int len = Compression::multibyte_read(fst);
  transducer.read(fst);

  FILE *input = stdin;
  FILE *output = stdout;

//  State* state = new State();
//  State current_state = *state; //points to initial state

//  Node* initial_node = new Node(); //points to the initial node of the transducer
//  state->init(initial_node);

//  state->step('t');
//  cout << "state size = " << state->size() << endl;


//  Node* final_node = new Node();
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

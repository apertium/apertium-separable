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

  LtLocale::tryToSetLocale();

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

  wchar_t in;
//  wcin.get(in); //get one character from stdin

  int line_number = 0;
  bool accepted = true;

  while (true) 
  {
    if (accepted) {
      line_number++;
    }
    int current_state = -1;


//    set<Node *> final_nodes = set<final_node>;
//    if (state->isFinal(final_nodes) == true) {
//      cout << line_number << "    " << input;
//      accepted = true;
//    }
  }

  return 0;
}

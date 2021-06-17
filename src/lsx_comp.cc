#include <cstdio>
#include <cerrno>
#include <iostream>
#include <stdlib.h>
#include <cstring>

#include <lsx_compiler.h>
#include <lttoolbox/lt_locale.h>

void endProgram(char *name)
{
    if(name != NULL)
    {
        cout << "USAGE: " << name << " [direction] dictionary_file(s) output_bin_file" << endl;
        cout << "Directions:" << endl;
        cout << "  lr:     left-to-right compilation" << endl;
        cout << "  rl:     right-to-left compilation" << endl;
    }
    exit(EXIT_FAILURE);
}

int main (int argc, char** argv)
{
  if(argc < 4)
  {
    endProgram(argv[0]);
  }

  LtLocale::tryToSetLocale();

  Compiler c;

  UString dir;

  if(strcmp(argv[1], "lr") == 0)
  {
    dir = Compiler::COMPILER_RESTRICTION_LR_VAL;
    c.parse(argv[2], Compiler::COMPILER_RESTRICTION_LR_VAL);
  }
  else if(strcmp(argv[1], "rl") == 0)
  {
    dir = Compiler::COMPILER_RESTRICTION_RL_VAL;
    c.parse(argv[2], Compiler::COMPILER_RESTRICTION_RL_VAL);
  }
  else
  {
    endProgram(argv[0]);
  }

  for(int i = 2; i < argc-1; i++)
  {
    c.parse(argv[i], dir);
  }

  FILE* fst = fopen(argv[argc-1], "w+");
  if(!fst)
  {
    cerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }
  c.write(fst);
  fclose(fst);

  return 0;
}

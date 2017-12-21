#include <cstdio>
#include <cerrno>
#include <iostream>
#include <stdlib.h>

#include <lsx_compiler.h>
#include <lttoolbox/lt_locale.h>

void endProgram(char *name)
{
    if(name != NULL)
    {
        wcout << "USAGE: " << name << " [direction] dictionary_file output_bin_file" << endl;
        wcout << "Directions:" << endl;
        wcout << "  lr:     left-to-right compilation" << endl;
        wcout << "  rl:     right-to-left compilation" << endl;
    }
    exit(EXIT_FAILURE);
}

int main (int argc, char** argv)
{
  if(argc != 4)
  {
    endProgram(argv[0]);
  }

  LtLocale::tryToSetLocale();

  Compiler c;

  if(strcmp(argv[1], "lr") == 0)
  {
    c.parse(argv[2], Compiler::COMPILER_RESTRICTION_LR_VAL);
  }
  else if(strcmp(argv[1], "rl") == 0)
  {
    c.parse(argv[2], Compiler::COMPILER_RESTRICTION_RL_VAL);
  }
  else
  {
    endProgram(argv[0]);
  }

  FILE* fst = fopen(argv[3], "w+");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }
  c.write(fst);
  fclose(fst);

  return 0;
}
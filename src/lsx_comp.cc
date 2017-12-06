#include <cstdio>
#include <cerrno>
#include <iostream>
#include <stdlib.h>

#include <lsx_compiler.h>
#include <lttoolbox/lt_locale.h>

int main (int argc, char** argv)
{
  if(argc != 3)
  {
    cout << "./lsx-comp <dix file> <bin file>" << endl;
    exit(0);
  }
  LtLocale::tryToSetLocale();

  Compiler c;
  c.parse(argv[1], L"LR");

  FILE* fst = fopen(argv[2], "w+");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }
  c.write(fst);
  fclose(fst);
  return 0;
}

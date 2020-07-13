#include <lttoolbox/lt_locale.h>
#include <iostream>
#include "lsx_processor.h"

int main (int argc, char** argv)
{
  char *fname = NULL;
  bool nullFlush = false;
  if(argc != 2 && argc != 3)
  {
    wcout << L"usage: ./lsx-proc [-z] <bin file>" << endl;
    exit(0);
  }
  if(argc == 2)
  {
    fname = argv[1];
  }
  else if(argc == 3)
  {
    nullFlush = true;
    fname = argv[2];
  }

  FILE *input = stdin;
  FILE *output = stdout;

  LSXProcessor fstp;

  LtLocale::tryToSetLocale();

  FILE *fst = fopen(fname, "r");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fname << "'." << endl;
    exit(EXIT_FAILURE);
  }
  if(nullFlush)
  {
    fstp.setNullFlush(true);
  }
  fstp.load(fst);
  fstp.process(input, output);

  return 0;
}

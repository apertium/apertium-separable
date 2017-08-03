#include <lttoolbox/lt_locale.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/state.h>

#include <lsx_FSTProcessor.cc>

int main (int argc, char** argv)
{
  if(argc != 2)
  {
    wcout << L"usage: ./lsx-comp <bin file>" << endl;
    exit(0);
  }

  FILE *input = stdin;
  FILE *output = stdout;

  FSTProcessor fstp;

  LtLocale::tryToSetLocale();

  FILE *fst = fopen(argv[1], "r");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }

  fstp.load(fst);
  fstp.initGeneration();
  fstp.lsx(input, output);

  return 0;
}
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <getopt.h>
#include <libgen.h>

#include <lsx_compiler.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/file_utils.h>

void endProgram(char *name)
{
  if(name != NULL) {
    cout << "USAGE: " << basename(name) << " [ -v ] [direction] dictionary_file(s) output_bin_file" << endl;
    cout << "  -v, --var:           specify language variant" << endl;
    cout << "  -h, --help:          print this help and exit" << endl;
    cout << "Directions:" << endl;
    cout << "  lr:     left-to-right compilation" << endl;
    cout << "  rl:     right-to-left compilation" << endl;
  }
  exit(EXIT_FAILURE);
}

int main (int argc, char** argv)
{
  LtLocale::tryToSetLocale();

  Compiler c;

#if HAVE_GETOPT_LONG
  int option_index = 0;
  static struct option long_options[] =
    {
     {"var",    required_argument, 0, 'v'},
     {"help",   no_argument,       0, 'h'},
     {0, 0, 0, 0}
    };
#endif
  while (true) {
#if HAVE_GETOPT_LONG
    int cnt=getopt_long(argc, argv, "v:h", long_options, &option_index);
#else
    int cnt=getopt(argc, argv, "v:h");
#endif
    if (cnt == -1) break;

    switch (cnt) {
    case 'v':
      c.setVariantValue(to_ustring(optarg));
      break;

    case 'h':
    default:
      endProgram(argv[0]);
      break;
    }
  }

  UString dir;

  if(strcmp(argv[optind], "lr") == 0)
  {
    dir = Compiler::COMPILER_RESTRICTION_LR_VAL;
  }
  else if(strcmp(argv[optind], "rl") == 0)
  {
    dir = Compiler::COMPILER_RESTRICTION_RL_VAL;
  }
  else
  {
    endProgram(argv[0]);
  }

  bool parsed = false;
  for(int i = optind+1; i < argc-1; i++)
  {
    parsed = true;
    c.parse(argv[i], dir);
  }
  if (!parsed) {
    endProgram(argv[0]);
  }

  FILE* fst = openOutBinFile(argv[argc-1]);
  c.write(fst);
  fclose(fst);

  return 0;
}

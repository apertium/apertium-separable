#include <lttoolbox/lt_locale.h>
#include <iostream>
#include <getopt.h>

#include "lsx_processor.h"

using namespace std;

void endProgram(char* name)
{
  cout << basename(name) << ": re-tokenize a stream" << endl;
  cout << "USAGE: " << basename(name) << " [ -w | -z ] fst_file [input_file [output_file]]" << endl;
#if HAVE_GETOPT_LONG
  cout << "  -w, --dictionary-case:   use dictionary case instead of surface case" << endl;
  cout << "  -z, --null-flush:        flush output on the null character" << endl;
  cout << "  -h, --help:              show this message" << endl;
#else
  cout << "  -w:   use dictionary case instead of surface case" << endl;
  cout << "  -z:   flush output on the null character" << endl;
  cout << "  -h:   show this message" << endl;
#endif
  exit(EXIT_FAILURE);
}

int main (int argc, char** argv)
{
  LtLocale::tryToSetLocale();
  
  LSXProcessor fstp;
  FILE* input = stdin;
  FILE* output = stdout;

#if HAVE_GETOPT_LONG
  static struct option long_options[]=
    {
     {"dictionary-case",  0, 0, 'w'},
     {"null-flush",       0, 0, 'z'},
     {"help"              0, 0, 'h'}
    };
#endif
  while (true) {
#if HAVE_GETOPT_LONG
    int option_index;
    int c = getopt_long(argc, argv, "wzh", long_options, &option_index);
#else
    int c = getopt(argc, argv, "wzh");
#endif

    if (c == -1) {
      break;
    }

    switch (c) {
    case 'w':
      fstp.setDictionaryCaseMode(true);
      break;

    case 'z':
      fstp.setNullFlush(true);
      break;

    case 'h':
    default:
      endProgram(argv[0]);
      break;
    }
  }

  if (optind > (argc - 1) || optind < (argc - 3)) {
    endProgram(argv[0]);
  }
  FILE* fst = fopen(argv[optind], "rb");
  if(!fst) {
    wcerr << "Error: Cannot open file '" << argv[optind] << "' for reading." << endl;
    exit(EXIT_FAILURE);
  }
  fstp.load(fst);

  if (optind <= (argc - 2)) {
    input = fopen(argv[optind+1], "rb");
    if (input == NULL || ferror(input)) {
      wcerr << "Error: Cannot open file '" << argv[optind+1] << "' for reading." << endl;
      exit(EXIT_FAILURE);
    }
  }
  if (optind <= (argc - 3)) {
    output = fopen(argv[optind+2], "wb");
    if (output == NULL || ferror(output)) {
      wcerr << "Error: Cannot open file '" << argv[optind+2] << "' for writing." << endl;
    }
  }
  
  fstp.process(input, output);

  return 0;
}

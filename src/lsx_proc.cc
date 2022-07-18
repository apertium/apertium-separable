#include <lttoolbox/lt_locale.h>
#include <lttoolbox/file_utils.h>
#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "lsx_processor.h"

using namespace std;

void endProgram(char* name)
{
  cout << basename(name) << ": re-tokenize a stream" << endl;
  cout << "USAGE: " << basename(name) << " [ -r | -w | -z ] fst_file [input_file [output_file]]" << endl;
#if HAVE_GETOPT_LONG
  cout << "  -r, --repeat:            continue applying rules until they match" << endl;
  cout << "  -w, --dictionary-case:   use dictionary case instead of surface case" << endl;
  cout << "  -z, --null-flush:        flush output on the null character" << endl;
  cout << "  -h, --help:              show this message" << endl;
#else
  cout << "  -r:   continue applying rules until they match" << endl;
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
  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);

#if HAVE_GETOPT_LONG
  static struct option long_options[]=
    {
     {"repeat",           0, 0, 'r'},
     {"dictionary-case",  0, 0, 'w'},
     {"null-flush",       0, 0, 'z'},
     {"help"              0, 0, 'h'}
    };
#endif
  while (true) {
#if HAVE_GETOPT_LONG
    int option_index;
    int c = getopt_long(argc, argv, "rwzh", long_options, &option_index);
#else
    int c = getopt(argc, argv, "rwzh");
#endif

    if (c == -1) {
      break;
    }

    switch (c) {
    case 'r':
      fstp.setRepeatMode(true);
      break;

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
  FILE* fst = openInBinFile(argv[optind]);
  fstp.load(fst);

  if (optind <= (argc - 2)) {
    input.open_or_exit(argv[optind+1]);
  }
  if (optind <= (argc - 3)) {
    output = openOutTextFile(argv[optind+2]);
  }
  
  fstp.process(input, output);

  u_fclose(output);
  return EXIT_SUCCESS;
}

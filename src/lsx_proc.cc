#include <lttoolbox/lt_locale.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/cli.h>

#include "lsx_processor.h"

using namespace std;

int main (int argc, char** argv)
{
  LtLocale::tryToSetLocale();
  CLI cli("re-tokenize a stream", PACKAGE_VERSION);
  cli.add_bool_arg('r', "repeat", "continue applying rules until they match");
  cli.add_bool_arg('w', "dictionary-case", "use dictionary case instead of surface case");
  cli.add_bool_arg('z', "null-flush", "flush output on the null character");
  cli.add_bool_arg('h', "help", "print this message and exit");
  cli.add_file_arg("fst_file", false);
  cli.add_file_arg("input_file", true);
  cli.add_file_arg("output_file", true);
  cli.parse_args(argc, argv);

  LSXProcessor fstp;

  fstp.setRepeatMode(cli.get_bools()["repeat"]);
  fstp.setDictionaryCaseMode(cli.get_bools()["dictionary-case"]);
  fstp.setNullFlush(cli.get_bools()["null-flush"]);

  FILE* fst = openInBinFile(cli.get_files()[0]);
  fstp.load(fst);
  fclose(fst);

  InputFile input;
  if (!cli.get_files()[1].empty()) {
    input.open_or_exit(cli.get_files()[1].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[2]);

  fstp.process(input, output);

  u_fclose(output);
  return EXIT_SUCCESS;
}

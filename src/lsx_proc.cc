#include <lttoolbox/lt_locale.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/i18n.h>

#include "lsx_processor.h"

using namespace std;

int main (int argc, char** argv)
{
  I18n i18n {ASP_I18N_DATA, "asp"};
  LtLocale::tryToSetLocale();
  CLI cli(i18n.format("lsx_proc_desc"), PACKAGE_VERSION);
  cli.add_bool_arg('p', "postgen", i18n.format("postgen_desc"));
  cli.add_bool_arg('r', "repeat", i18n.format("repeat_desc"));
  cli.add_bool_arg('w', "dictionary-case", i18n.format("dictionary_case_desc"));
  cli.add_bool_arg('z', "null-flush", i18n.format("null_flush_desc"));
  cli.add_bool_arg('h', "help", i18n.format("help_desc"));
  cli.add_file_arg("fst_file", false);
  cli.add_file_arg("input_file", true);
  cli.add_file_arg("output_file", true);
  cli.parse_args(argc, argv);

  LSXProcessor fstp;

  fstp.setPostgenMode(cli.get_bools()["postgen"]);
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

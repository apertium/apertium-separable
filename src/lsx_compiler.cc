#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
// #include <lttoolbox/trans_exe.h>

#include <lttoolbox/compiler.h>
// #include <compiler_copy.cc>
#include <lttoolbox/xml_parse_util.h>

using namespace std;

int main (int argc, char** argv) {
    Alphabet alphabet;
    Transducer t;

    LtLocale::tryToSetLocale();

    Compiler c;
    c.parse(argv[1], L"lr");

    FILE* fst = fopen("lsx-compiler.fst", "w+");
    c.write(fst);

    fclose(fst);

    return 0;
}

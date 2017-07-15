#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <stdlib.h>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>

#include <lttoolbox/compiler.h>
#include <lttoolbox/xml_parse_util.h>

using namespace std;

int main (int argc, char** argv) 
{
    Alphabet alphabet;
    Transducer t;

    LtLocale::tryToSetLocale();

    if(argc < 3) 
    {
      wcout << L"lsx-comp <dix file> <bin file>" << endl;
      exit(0);
    }

    Compiler c;
    c.parse(argv[1], L"lr");

    FILE* fst = fopen(argv[2], "w+");
    c.write(fst);

    fclose(fst);

    return 0;
}

#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
// #include <fstream>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);


/* get the text between delim1 and delim2 */
/* next_token() */
wstring
readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc(input));
    result += c;
  }

  return result;
}


/***
main
***/

int main (int argc, char** argv)
{
  Alphabet alphabet;
  TransExe transducer;

  LtLocale::tryToSetLocale();


  FILE *fst = fopen(argv[1], "r");

  set<wchar_t> alphabetic_chars;
  int len = Compression::multibyte_read(fst);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(fst)));
    len--;
  }

  alphabet.read(fst);
  wcerr << L"alphabet_size: " << alphabet.size() << endl;

  len = Compression::multibyte_read(fst);

  len = Compression::multibyte_read(fst);
  wcerr << len << endl;
  wstring name = L"";
  while(len > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--;
  }
  wcerr << name << endl;

  transducer.read(fst, alphabet);

  FILE *input = stdin;
  FILE *output = stdout;

  vector<State> alive_states;
  set<Node *> anfinals;
  set<wchar_t> escaped_chars;

  escaped_chars.insert(L'[');
  escaped_chars.insert(L']');
  escaped_chars.insert(L'{');
  escaped_chars.insert(L'}');
  escaped_chars.insert(L'^');
  escaped_chars.insert(L'$');
  escaped_chars.insert(L'/');
  escaped_chars.insert(L'\\');
  escaped_chars.insert(L'@');
  escaped_chars.insert(L'<');
  escaped_chars.insert(L'>');

  State *initial_state;
  initial_state = new State();
  initial_state->init(transducer.getInitial());
  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

  /*
  processing
  */

  vector<State> new_states;

  alive_states.push_back(*initial_state);

  bool outOfWord = true;
  bool isEscaped = false;

  State s = *initial_state;

  s.step('t');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('a');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('k');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('e');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<vblex>"), alphabet(L"<ANY_TAG>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<ANY_TAG>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<ANY_TAG>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<ANY_TAG>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<$>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('o');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('u');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step('t');
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<ANY_TAG>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;
  s.step(alphabet(L"<$>"));
  wcerr << s.size() << L" ||| " << s.getReadableString(alphabet) << L" ||| " << s.isFinal(anfinals) << endl;




  return 0;
}

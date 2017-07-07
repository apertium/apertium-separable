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
#include <lttoolbox/trans_exe.h>

int main (int argc, char** argv) {
  Alphabet alphabet;
  Transducer t;

  LtLocale::tryToSetLocale();

  alphabet.includeSymbol(L"<vblex>");
  alphabet.includeSymbol(L"<n>");
  alphabet.includeSymbol(L"<adj>");
  alphabet.includeSymbol(L"<det>");
  alphabet.includeSymbol(L"<prn>");
  alphabet.includeSymbol(L"<np>");
  alphabet.includeSymbol(L"<adv>");
  alphabet.includeSymbol(L"<pr>");

  alphabet.includeSymbol(L"<ANY_TAG>");
  alphabet.includeSymbol(L"<ANY_CHAR>");
  alphabet.includeSymbol(L"<$>");

  int vblex_sym = alphabet(L"<vblex>");
  int n_sym = alphabet(L"<n>");
  int adj_sym = alphabet(L"<adj>");
  int det_sym = alphabet(L"<det>");
  int prn_sym = alphabet(L"<prn>");
  int np_sym = alphabet(L"<np>");
  int adv_sym = alphabet(L"<adv>");
  int pr_sym = alphabet(L"<pr>");

  int any_tag = alphabet(L"<ANY_TAG>");
  int any_char = alphabet(L"<ANY_CHAR>");
  int wb_sym = alphabet(L"<$>");

  int initial = t.getInitial();
  int take_out = initial; //0
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(L't',L't'), take_out); //1
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(L'a',L'a'), take_out); //2
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(L'k',L'k'), take_out); //3
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(L'e',L'e'), take_out); //4
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(0,L'#'), take_out); //5
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(0,L' '), take_out); //6
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(0,L'o'), take_out); //7
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(0,L'u'), take_out); //8
  cout << take_out << endl;
  take_out = t.insertSingleTransduction(alphabet(0,L't'), take_out); //9
  cout << take_out << endl;

  take_out = t.insertSingleTransduction(alphabet(vblex_sym, vblex_sym), take_out);
  cout << take_out << endl;
  int loop = take_out; //stores the src state int
  take_out = t.insertSingleTransduction(alphabet(any_tag, any_tag), loop);
  t.linkStates(take_out, loop, 0);

  take_out = t.insertSingleTransduction(alphabet(wb_sym, wb_sym), take_out);
  take_out = t.insertSingleTransduction(alphabet(' ', ' '), take_out);

  loop = take_out;
  take_out = t.insertSingleTransduction(alphabet(any_char, any_char), loop);
  t.linkStates(take_out, loop, 0);

  take_out = t.insertSingleTransduction(alphabet(n_sym, n_sym), take_out;
  loop = take_out;
  take_out = t.insertSingleTransduction(alphabet(any_tag, any_tag), loop);
  t.linkStates(take_out, loop, 0);

  take_out = t.insertSingleTransduction(alphabet(wb_sym, wb_sym), take_out);
  take_out = t.insertSingleTransduction(alphabet(' ', ' '), take_out);


  take_out = t.insertSingleTransduction(alphabet(L'o',0), take_out);
  take_out = t.insertSingleTransduction(alphabet(L'u',0), take_out);
  take_out = t.insertSingleTransduction(alphabet(L't',0), take_out);

  take_out = t.insertSingleTransduction(alphabet(any_tag, 0), take_out);
  take_out = t.insertSingleTransduction(alphabet(wb_sym,0), take_out);

  t.setFinal(take_out);

  FILE* fst = fopen("takeout.fst", "w+");

  // First write the letter symbols of the alphabet
  Compression::wstring_write(L"aekout", fst);
  // Then write the multicharacter symbols
  alphabet.write(fst);
  // Then write then number of transducers
  Compression::multibyte_write(1, fst);
  // Then write the name of the transducer
  Compression::wstring_write(L"main@standard", fst);
  // Then write the transducer
  t.write(fst);
  wcout << t.size() << endl ;
  fclose(fst);

  return 0;
}

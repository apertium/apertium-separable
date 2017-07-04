#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <set>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
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
  alphabet.includeSymbol(L"&");
  alphabet.includeSymbol(L"$");

  int vblex_sym = alphabet(L"<vblex>");
  int n_sym = alphabet(L"<n>");
  int adj_sym = alphabet(L"<adj>");
  int det_sym = alphabet(L"<det>");
  int prn_sym = alphabet(L"<prn>");
  int np_sym = alphabet(L"<np>");
  int adv_sym = alphabet(L"<adv>");
  int pr_sym = alphabet(L"<pr>");

  int initial = t.getInitial();
  // int current_state = initial;

  int take_out = initial;
  take_out = t.insertSingleTransduction(alphabet(L't',L't'), take_out);
  take_out = t.insertSingleTransduction(alphabet(L'a',L'a'), take_out);
  take_out = t.insertSingleTransduction(alphabet(L'k',L'k'), take_out);
  take_out = t.insertSingleTransduction(alphabet(L'e',L'e'), take_out);

  take_out = t.insertSingleTransduction(alphabet(L' ',L' '), take_out);

  take_out = t.insertSingleTransduction(alphabet(L'o',L'o'), take_out);
  take_out = t.insertSingleTransduction(alphabet(L'u',L'u'), take_out);
  take_out = t.insertSingleTransduction(alphabet(L't',L't'), take_out);

  t.setFinal(take_out);
  // take_out = t.insertSingleTransduction(alphabet(L'^',L'^'), take_out);
  // take_out = t.insertSingleTransduction(alphabet(L'&',L'&'), take_out);
  // take_out = t.insertSingleTransduction(alphabet(L'$',L'$'), take_out);

  FILE* fst = fopen("takeout.fst", "w");
  t.write(fst);
  fclose(fst);

  // fst = fopen("takeout.fst", "r");
  //
  // TransExe te;
  // te.read(fst, alphabet);
  // fclose(fst);
  //
  // State *initial_state = new State();
  // initial_state->init(te.getInitial());
  // State current_state = *initial_state;

  // cout << initial << endl;
  // cout << "running" << endl;
  return 0;
}
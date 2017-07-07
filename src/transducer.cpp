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
    take_out = t.insertSingleTransduction(alphabet(L't',L't'), take_out);
    take_out = t.insertSingleTransduction(alphabet(L'a',L'a'), take_out);
    take_out = t.insertSingleTransduction(alphabet(L'k',L'k'), take_out);
    take_out = t.insertSingleTransduction(alphabet(L'e',L'e'), take_out);
    take_out = t.insertSingleTransduction(alphabet(0,L'#'), take_out);
    take_out = t.insertSingleTransduction(alphabet(0,L' '), take_out);
    take_out = t.insertSingleTransduction(alphabet(0,L'o'), take_out);
    take_out = t.insertSingleTransduction(alphabet(0,L'u'), take_out);
    take_out = t.insertSingleTransduction(alphabet(0,L't'), take_out);
    take_out = t.insertSingleTransduction(alphabet(vblex_sym,vblex_sym), take_out);
    int loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);
    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    /* noun phrase acceptor:
    n
    prn
    det.* n.*
    adj. n.*
    adj.* n.*
    det.* adj n.*
    adj.* adj.* n.*
    prn.pers.*
    prn.dem.*
    */
    int after_takeout = take_out;

    /* no np */
    take_out = after_takeout;
    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(0,0), loop);
    t.linkStates(take_out, loop, 0);

    /* first lemma in the np */
    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    /* no modifier */
    int modifier = take_out;

    take_out = modifier;
    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(0,0), loop);
    t.linkStates(take_out, loop, 0);
    int from_nomod = take_out;

    /* det */
    take_out = modifier;
    take_out = t.insertSingleTransduction(alphabet(det_sym,det_sym), take_out);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    int from_det = take_out;

    /* adj */
    take_out = modifier;
    take_out = t.insertSingleTransduction(alphabet(adj_sym,adj_sym), take_out);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    //may not have a second tag
    take_out = t.insertSingleTransduction(alphabet(0,0), take_out);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);
    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out-1);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    int from_adj = take_out;

    /* n */
    take_out = from_nomod;
    take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);

    take_out = from_det;
    take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);

    take_out = from_adj;
    take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    /* out */
    take_out = t.insertSingleTransduction(alphabet(L'o',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(L'u',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(L't',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(any_tag, 0), take_out);
    take_out = t.insertSingleTransduction(alphabet(wb_sym,0), take_out);

    /* pr */
    // take_out = reset;
    //
    // loop = take_out;
    // int none = take_out;
    // take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    // none = t.insertSingleTransduction(alphabet(0,0), none);
    // t.linkStates(take_out, loop, 0);
    // t.linkStates(none, loop, 0);
    //
    // take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);
    // none = t.insertSingleTransduction(alphabet(0,0), none);
    //
    // loop = take_out;
    // none = take_out;
    // take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    // none = t.insertSingleTransduction(alphabet(0,0), none);
    // t.linkStates(take_out, loop, 0);
    // t.linkStates(none, loop, 0);
    //
    // take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);
    // none = t.insertSingleTransduction(alphabet(0,0), none);
    //
    // take_out = t.insertSingleTransduction(alphabet(L'o',0), take_out);
    // take_out = t.insertSingleTransduction(alphabet(L'u',0), take_out);
    // take_out = t.insertSingleTransduction(alphabet(L't',0), take_out);
    // take_out = t.insertSingleTransduction(alphabet(any_tag, 0), take_out);
    // take_out = t.insertSingleTransduction(alphabet(wb_sym,0), take_out);




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
    wcout << "t.size(): " << t.size() << endl ;
    fclose(fst);

    return 0;
}
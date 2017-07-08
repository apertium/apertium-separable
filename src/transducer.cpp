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
    np
    */
    int after_takeout = take_out;

    /* no det */
    int from_nodet = after_takeout;

    /* first lemma */
    loop = after_takeout;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    int first_lm = take_out;

    /* prn */
    take_out = t.insertSingleTransduction(alphabet(prn_sym,prn_sym), first_lm);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    int after_prn = take_out;

    /* np */
    take_out = t.insertSingleTransduction(alphabet(np_sym,np_sym), first_lm);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    int after_np = take_out;

    /* det */
    take_out = t.insertSingleTransduction(alphabet(det_sym,det_sym), first_lm);

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    int after_det = take_out;

    /* no adj */
    int from_noadj = take_out; //same as after_det

    /* lemma for the adj */
    loop = after_det;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    int lm_adj = take_out;

    /* adj */
    take_out = t.insertSingleTransduction(alphabet(adj_sym,adj_sym), lm_adj);

    int optional_adj = take_out;

    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    //may not have a second tag
    t.linkStates(optional_adj, take_out, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    int after_adj = take_out;

    /* no n */
    int from_non = take_out; //same as after_adj

    /* lemma for the noun */
    loop = after_adj;
    take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
    t.linkStates(take_out, loop, 0);

    int lm_noun = take_out;

    /* possible subsequent adj */
    t.linkStates(lm_noun, lm_adj, alphabet(adj_sym,adj_sym));

    /* n */
    take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), lm_noun);

    // take_out = after_det;
    // take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);
    //
    // take_out = from_adj;
    // take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), take_out);
    //
    loop = take_out;
    take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
    t.linkStates(take_out, loop, 0);

    take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

    int after_n = take_out;

    /* out */
    int before_out = take_out;

    take_out = t.insertSingleTransduction(alphabet(L'o',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(L'u',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(L't',0), take_out);
    take_out = t.insertSingleTransduction(alphabet(any_tag, 0), take_out);
    take_out = t.insertSingleTransduction(alphabet(wb_sym,0), take_out);

    t.setFinal(take_out);

    /* final link states */
    t.linkStates(after_takeout, before_out, 0);
    t.linkStates(after_prn, before_out, 0);
    t.linkStates(after_np, before_out, 0);
    t.linkStates(from_nodet, after_det, 0);
    t.linkStates(from_noadj, after_adj, 0);


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
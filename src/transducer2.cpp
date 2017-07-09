#include <cwchar>
#include <cstdio>
#include <cerrno>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <regex>

#include <lttoolbox/ltstr.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/transducer.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

using namespace std;

int main (int argc, char** argv) {
    Alphabet alphabet;

    LtLocale::tryToSetLocale();

    alphabet.includeSymbol(L"<vblex>");
    alphabet.includeSymbol(L"<n>");
    alphabet.includeSymbol(L"<adj>");
    alphabet.includeSymbol(L"<det>");
    alphabet.includeSymbol(L"<prn>");
    alphabet.includeSymbol(L"<np>");

    alphabet.includeSymbol(L"<ANY_TAG>");
    alphabet.includeSymbol(L"<ANY_CHAR>");
    alphabet.includeSymbol(L"<$>");

    int vblex_sym = alphabet(L"<vblex>");
    int n_sym = alphabet(L"<n>");
    int adj_sym = alphabet(L"<adj>");
    int det_sym = alphabet(L"<det>");
    int prn_sym = alphabet(L"<prn>");
    int np_sym = alphabet(L"<np>");

    int any_tag = alphabet(L"<ANY_TAG>");
    int any_char = alphabet(L"<ANY_CHAR>");
    int wb_sym = alphabet(L"<$>");

    /* reap from input file */
    for (string line; getline(cin, line);) {
        Transducer t;
        string first_token = line.substr(0, line.find(' '));
        string second_token = line.substr(line.find(' ') + 1);

        /* noun phrase acceptor: see README */

        int initial = t.getInitial();
        int take_out = initial;
        for (wchar_t c : first_token) {
            take_out = t.insertSingleTransduction(alphabet(c,c), take_out);
        }
        take_out = t.insertSingleTransduction(alphabet(0,L'#'), take_out);
        take_out = t.insertSingleTransduction(alphabet(0,L' '), take_out);
        for (wchar_t c : second_token) {
            take_out = t.insertSingleTransduction(alphabet(0,c), take_out);
        }
        take_out = t.insertSingleTransduction(alphabet(vblex_sym,vblex_sym), take_out);
        int loop = take_out;
        take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
        t.linkStates(take_out, loop, 0);
        take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

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

        /* lemma for the noun */
        loop = after_adj;
        take_out = t.insertSingleTransduction(alphabet(any_char,any_char), loop);
        t.linkStates(take_out, loop, 0);

        int lm_noun = take_out;

        /* possible subsequent adj */
        t.linkStates(lm_noun, lm_adj, alphabet(adj_sym,adj_sym));

        /* n */
        take_out = t.insertSingleTransduction(alphabet(n_sym,n_sym), lm_noun);

        loop = take_out;
        take_out = t.insertSingleTransduction(alphabet(any_tag,any_tag), loop);
        t.linkStates(take_out, loop, 0);

        take_out = t.insertSingleTransduction(alphabet(wb_sym,wb_sym), take_out);

        /* out */
        int before_out = take_out;

        for (wchar_t c : second_token) {
            take_out = t.insertSingleTransduction(alphabet(c,0), take_out);
        }
        take_out = t.insertSingleTransduction(alphabet(any_tag, 0), take_out);
        take_out = t.insertSingleTransduction(alphabet(wb_sym,0), take_out);

        t.setFinal(take_out);

        /* final link states */
        t.linkStates(after_takeout, before_out, 0);
        t.linkStates(after_prn, before_out, 0);
        t.linkStates(after_np, before_out, 0);
        t.linkStates(from_nodet, after_det, 0);
        t.linkStates(from_noadj, after_adj, 0);

        string filename = regex_replace(line,std::regex("\\s+"), "") + ".fst";
        FILE* fst = fopen(filename.c_str(), "w+");
        // First write the letter symbols of the alphabet
        Compression::wstring_write(L"abcdefghijklmnopqrstuvwxyz", fst);
        // Then write the multicharacter symbols
        alphabet.write(fst);
        // Then write then number of transducers
        Compression::multibyte_write(1, fst);
        // Then write the name of the transducer
        Compression::wstring_write(L"main@standard", fst);
        // Then write the transducer
        t.write(fst);
        cout << line << " t.size(): " << t.size() << endl ;
        fclose(fst);
    }

    return 0;
}
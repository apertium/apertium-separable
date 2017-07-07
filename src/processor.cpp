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
        c = static_cast<wchar_t>(fgetwc(input)); //fget_unlocked
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
    wcout << L"alphabet_size: " << alphabet.size() << endl;

    len = Compression::multibyte_read(fst);
    len = Compression::multibyte_read(fst);
    wcout << len << endl;
    wstring name = L"";
    while(len > 0)
    {
        name += static_cast<wchar_t>(Compression::multibyte_read(fst));
        len--;
    }
    wcout << name << endl;

    transducer.read(fst, alphabet);

    FILE *input = stdin;
    FILE *output = stdout;

    /* preparing for processing */
    vector<State> alive_states; //A set of alive states is maintained to compute all the possible ways to
    set<Node *> anfinals; //alive node finals ?
    set<wchar_t> escaped_chars;

    State* initial_state = new State();
    initial_state->init(transducer.getInitial()); // getInitial() returns an int
    anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());

    set<int> final_states = transducer.getFinals();
    for(auto final_state : final_states) {
        final_state.init(transducer.getInitial()); //initialize
    }


    /* processing */

    vector<State> new_states;
    alive_states.push_back(*initial_state);
    // TODO: insert the other states
    // TODO: insert the final state

    int line_number = 0;
    bool accepted = true;
    while(!feof(input)) // while true
    {
        //initialize conditions
        int tag_count = 0;
        State* current_state = initial_state;
        bool in_lemma = false;
        bool in_take = false;
        bool in_out = false;

        while (alive_states.size() > 1 and !isFinal(current_state)) {
            //get the next token
            int val = fgetwc(input); // read 1 wide char
            bool is_tag = false;
            if(val == L'<') // if in tag, get the whole tag
            {
                in_lemma = false;
                is_tag = true;
                wstring tag = L"";
                tag = readFullBlock(input, L'<', L'>');
                val = static_cast<int>(alphabet(tag));

                tag_count++;

                cout << "val before: " << val << endl;
                cout << "tag_count: " << tag_count << endl;

                if(val == 0 && tag_count > 2) //TODO: val==0?
                {
                    val = static_cast<int>(alphabet(L"<ANY_TAG>"));
                }

                cout << "val after: " << val << endl;
                fwprintf(stderr, L"tag %S: %d\n", tag.c_str(), val);

                if (tag == '<sent>') {
                    accepted = true;
                }
            }
            else if(in_lemma && !in_take && !in_out) {
                val == static_cast<int>(alphabet(L"&"));
            }

            // if (current_state == initial_state && not eof) {
                //successfully reached eof
                //exit()

            if (current_state == initial_state && val != '\n') {
                accepted = true;
                break;
            } else if (val == '\n') { //or sent
                accepted = true;
            }

            //step into the next state
            for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++) { //step //for every state in alive_states
                State s = *it;

                if (tag_count > 2) {
                    s.step(val, alphabet(L"<ANY_TAG>"));
                } else {
                    s.step(val)
                }

                if(s.size() > 0)
                {
                    new_states.push_back(s);
                }
                wcout << (wchar_t) val << L" " << L"size: " << s.size() << L" final: " << s.isFinal(anfinals) << endl;
            }

            alive_states.swap(new_states);
        }
        return 0;
    }

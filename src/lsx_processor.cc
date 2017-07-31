#include <lttoolbox/lt_locale.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/trans_exe.h>
#include <lttoolbox/state.h>
#include <lttoolbox/fst_processor.h>

wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);

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

int main (int argc, char** argv)
{
  if(argc != 2)
  {
    wcout << L"./lsx-comp <bin file>" << endl;
    exit(0);
  }

  FILE *input = stdin;
  FILE *output = stdout;

  /*
  Alphabet alphabet;
  TransExe transducer;
  */
  FSTProcessor fstp;

  LtLocale::tryToSetLocale();

  FILE *fst = fopen(argv[1], "r");
  if(!fst)
  {
    wcerr << "Error: Cannot open file '" << fst << "'." << endl;
    exit(EXIT_FAILURE);
  }

  /*
  set<wchar_t> alphabetic_chars;
  int len = Compression::multibyte_read(fst);
  while(len > 0)
  {
    alphabetic_chars.insert(static_cast<wchar_t>(Compression::multibyte_read(fst)));
    len--;
  }

  alphabet.read(fst);

  len = Compression::multibyte_read(fst);
  len = Compression::multibyte_read(fst);

  wstring name = L"";
  while(len > 0)
  {
    name += static_cast<wchar_t>(Compression::multibyte_read(fst));
    len--;
  }

  transducer.read(fst, alphabet);
  */

  fstp.load(fst);

  /*
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
  */

  /*
  State *initial_state = new State();
  initial_state->init(transducer.getInitial());
  set<Node *> anfinals;
  anfinals.insert(transducer.getFinals().begin(), transducer.getFinals().end());
  */

  fstp.initGeneration();
  fstp.lsx(input, output);
  //
  // vector<State> new_states;
  // vector<State> alive_states;
  // list<wstring> blankqueue;
  // wstring blank;
  // bool outOfWord = true;
  // // bool isEscaped = false;
  // bool finalFound = false;
  // wstring in = L"";
  // wstring out;
  //
  // alive_states.push_back(*initial_state);
  //
  //
  // while(!feof(input))
  // {
  //   int val = fgetwc(input);
  //
  //   if(alive_states.size() == 0 && !finalFound)
  //   {
  //     alive_states.push_back(*initial_state);
  //     fputws(in.c_str(), output);
  //     in = L"";
  //   }
  //   else if(alive_states.size() == 0 && finalFound)
  //   {
  //     in = L"";
  //     finalFound = false;
  //   }
  //
  //   if((val == L'^' && !isEscaped && outOfWord) || feof(input))
  //   {
  //     outOfWord = false;
  //     blankqueue.push_back(blank);
  //     blank = L"";
  //     // fputws(blankqueue.front().c_str(), output);
  //     // blankqueue.pop_front();
  //     in += val;
  //     continue;
  //   }
  //   if(outOfWord)
  //   {
  //     blank += val;
  //     continue;
  //   }
  //
  //   if((feof(input) || val == L'$') && !isEscaped && !outOfWord)
  //   {
  //     new_states.clear();
  //     for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
  //     {
  //       State s = *it;
  //       fflush(output);
  //       s.step(alphabet(L"<$>"));
  //       if(s.size() > 0)
  //       {
  //         new_states.push_back(s);
  //       }
  //
  //       if(s.isFinal(anfinals))
  //       {
  //         out += s.filterFinals(anfinals, alphabet, escaped_chars);
  //         new_states.push_back(*initial_state);
  //       }
  //     }
  //
  //     alive_states.swap(new_states);
  //     outOfWord = true;
  //     in += val;
  //
  //     if(alive_states.size() == 0)
  //     {
  //       // cout << "HERE";
  //       if(blankqueue.size() > 0)
  //       {
  //         fputws(blankqueue.front().c_str(), output);
  //         fflush(output);
  //         blankqueue.pop_front();
  //       }
  //     }
  //
  //     continue;
  //   }
  //
  //   if(!outOfWord)
  //   {
  //     if(val == L'<') // tag
  //     {
  //       wstring tag = fstp.readFullBlock(input, L'<', L'>');
  //       if(!alphabet.isSymbolDefined(tag))
  //       {
  //         alphabet.includeSymbol(tag);
  //       }
  //       val = static_cast<int>(alphabet(tag));
  //       in += tag;
  //     }
  //     else
  //     {
  //       in += (wchar_t) val;
  //     }
  //
  //     new_states.clear();
  //     wstring res = L"";
  //     for(vector<State>::const_iterator it = alive_states.begin(); it != alive_states.end(); it++)
  //     {
  //       res = L"";
  //       State s = *it;
  //       if(val < 0)
  //       {
  //         fflush(output);
  //         s.step_override(val, alphabet(L"<ANY_TAG>"), val);
  //       }
  //       else if(val > 0)
  //       {
  //         fflush(output);
  //         int val_lowercase = towlower(val);
  //         s.step_override(val_lowercase, alphabet(L"<ANY_CHAR>"), val); // FIXME deal with cases!
  //       }
  //
  //       if(s.size() > 0)
  //       {
  //         new_states.push_back(s);
  //       }
  //
  //       if(s.isFinal(anfinals))
  //       {
  //         out = s.filterFinals(anfinals, alphabet, escaped_chars);
  //         new_states.push_back(*initial_state);
  //         finalFound = true;
  //
  //         for (int i=0; i < (int) out.size(); i++)
  //         {
  //           wchar_t c = out[i];
  //           /* FIXME these hacks (?) */
  //           if(c == L'/')
  //           {
  //             out[i] = L'^';
  //           }
  //           else if(c == L'$' && out[i-1] == L'<' && out[i+1] == L'>')
  //           {
  //             out.erase(i+1, 1);
  //             out.erase(i-1, 1);
  //             break;
  //           }
  //         }
  //         out = out.substr(0, out.length()-3); // remove extra trailing
  //         // for(wchar_t& c : out)
  //         for(int i=0; i < (int) out.size(); i++)
  //         {
  //           // cout << blankqueue.size();
  //           if(out[i] == L'$' && blankqueue.size()>0)
  //           {
  //             out.insert(i+1, blankqueue.front().c_str());
  //             blankqueue.pop_front();
  //           }
  //         }
  //         fputws(out.c_str(), output);
  //       }
  //     }
  //     alive_states.swap(new_states);
  //   }
  //   else if(outOfWord) // FIXME need to deal with superblank stuff
  //   {
  //     fputwc(val, output);
  //     continue;
  //   }
  //   else
  //   {
  //     wcerr << L"outOfWord error" << endl;
  //   }
  // }
  //
  // // wcout << endl << endl << L"BQ size: " << blankqueue.size() << endl;
  // /* flushing rest of the blanks here */
  // for (wstring b : blankqueue)
  // {
  //   fputws(b.c_str(), output);
  //   // wcout << L"B" << b.c_str() << L"B" << endl;
  // }
  return 0;
}
#ifndef _LSX_PROCESSOR_H_
#define _LSX_PROCESSOR_H_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/my_stdio.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>
#include <deque>

class LSXProcessor
{
private:
  TransExe trans;
  State initial_state;
  set<wchar_t> escaped_chars;
  set<wchar_t> alphabetic_chars;
  map<Node *, double> all_finals;
  Alphabet alphabet;
  bool null_flush;
  bool dictionary_case;
  bool at_end;
  bool at_null;

  deque<wstring> blank_queue;
  deque<wstring> bound_blank_queue;
  deque<wstring> lu_queue;

  void readNextLU(FILE* input);
  void processWord(FILE* input, FILE* output);

  int word_boundary;
  int any_char;
  int any_tag;
public:
  LSXProcessor();
  void load(FILE* input);
  void process(FILE* input, FILE* output);
  void setNullFlush(bool val)
  {
    null_flush = val;
  }
  void setDictionaryCaseMode(bool val)
  {
    dictionary_case = val;
  }
};

#endif

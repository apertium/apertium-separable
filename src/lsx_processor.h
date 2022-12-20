#ifndef _LSX_PROCESSOR_H_
#define _LSX_PROCESSOR_H_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>
#include <unicode/ustdio.h>
#include <deque>

class LSXProcessor
{
private:
  Node root;
  std::map<UString, TransExe> trans;
  State initial_state;
  std::set<UChar32> escaped_chars;
  std::set<UChar32> alphabetic_chars;
  std::map<Node *, double> all_finals;
  Alphabet alphabet;
  bool null_flush = true;
  bool dictionary_case = false;
  bool repeat_rules = false;
  bool at_end = false;
  bool at_null = false;

  std::deque<UString> blank_queue;
  std::deque<UString> bound_blank_queue;
  std::deque<UString> lu_queue;

  void readNextLU(InputFile& input);
  void processWord(InputFile& input, UFILE* output);

  int word_boundary;
  int word_boundary_s;
  int word_boundary_ns;
  int any_char;
  int any_tag;
public:
  LSXProcessor();
  void load(FILE* input);
  void process(InputFile& input, UFILE* output);
  void setNullFlush(bool val)
  {
    null_flush = val;
  }
  void setDictionaryCaseMode(bool val)
  {
    dictionary_case = val;
  }
  void setRepeatMode(bool val)
  {
    repeat_rules = val;
  }
};

#endif

#ifndef _LSX_PROCESSOR_H_
#define _LSX_PROCESSOR_H_

#include <lttoolbox/alphabet_exe.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/my_stdio.h>
#include <lttoolbox/state.h>
#include <lttoolbox/string_writer.h>
#include <lttoolbox/transducer_exe.h>
#include <unicode/ustdio.h>
#include <deque>

class LSXProcessor
{
private:
  StringWriter str_write;
  TransducerExe trans;
  State initial_state;
  set<UChar32> escaped_chars;
  set<UChar32> alphabetic_chars;
  set<TransducerExe*> all_finals;
  AlphabetExe alphabet;
  bool null_flush;
  bool dictionary_case;
  bool at_end;
  bool at_null;

  bool mmapping = false;
  void* mmap_pointer;
  int mmap_len;

  deque<UString> blank_queue;
  deque<UString> bound_blank_queue;
  deque<UString> lu_queue;

  void readNextLU(InputFile& input);
  void processWord(InputFile& input, UFILE* output);

  int word_boundary;
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
};

#endif

/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FSTPROCESSOR_
#define _FSTPROCESSOR_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/my_stdio.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

#include <cwchar>
#include <map>
#include <queue>
#include <set>
#include <string>

using namespace std;

/**
 * Kind of output of the generator module
 */
enum GenerationMode
{
  gm_clean,      // clear all
  gm_unknown,    // display unknown words, clear transfer and generation tags
  gm_all,        // display all
  gm_tagged,     // tagged generation
  gm_tagged_nm,  // clean tagged generation
  gm_carefulcase // try lowercase iff no uppercase
};

/**
 * Class that implements the FST-based modules of the system
 */
class FSTProcessor
{
private:
  /**
   * Transducers in FSTP
   */
  map<wstring, TransExe, Ltstr> transducers;

  /**
   * Current state of lexical analysis
   */
  State *current_state;

  /**
   * Initial state of every token
   */
  State *initial_state;

  /**
   * Set of final states of inconditional sections in the dictionaries
   */
  set<Node *> inconditional;

  /**
   * Set of final states of standard sections in the dictionaries
   */
  set<Node *> standard;

  /**
   * Set of final states of postblank sections in the dictionaries
   */
  set<Node *> postblank;

  /**
   * Set of final states of preblank sections in the dictionaries
   */
  set<Node *> preblank;

  /**
   * Merge of 'inconditional', 'standard', 'postblank' and 'preblank' sets
   */
  set<Node *> all_finals;

  /**
   * Queue of blanks, used in reading methods
   */
  queue<wstring> blankqueue;

  /**
   * Set of characters being considered alphabetics
   */
  set<wchar_t> alphabetic_chars;

  /**
   * Set of characters to escape with a backslash
   */
  set<wchar_t> escaped_chars;

  /**
   * Alphabet
   */
  Alphabet alphabet;

  /**
   * Input buffer
   */
  Buffer<int> input_buffer;

  /**
   * true if the position of input stream is out of a word
   */
  bool outOfWord;

  /**
   * true if we're automatically removing surface forms.
   */
  bool biltransSurfaceForms;


  /**
   * if true, makes always difference between uppercase and lowercase
   * characters
   */
  bool caseSensitive;

  /**
   * if true, uses the dictionary case, discarding surface case
   * information
   */
  bool dictionaryCase;

  /**
   * if true, flush the output when the null character is found
   */
  bool nullFlush;

  /**
   * nullFlush property for the skipUntil function
   */
  bool nullFlushGeneration;

  /**
   * try analysing unknown words as compounds
   */
  bool do_decomposition;

  /**
   * Symbol of CompoundOnlyL
   */
  int compoundOnlyLSymbol;

  /**
   * Symbol of CompoundR
   */
  int compoundRSymbol;

  /**
   * Show or not the controls symbols (as compoundRSymbol)
   */
   bool showControlSymbols;

  /**
   * Max compound elements
   * Hard coded for now, but there might come a switch one day
   */
  int compound_max_elements;

  /**
   * Reads a block from the stream input, enclosed by delim1 and delim2
   * @param input the stream being read
   * @param delim1 the delimiter of the beginning of the sequence
   * @param delim1 the delimiter of the end of the sequence
   */
  wstring readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2);


  /**
   * Tests if a character is in the set of escaped_chars
   * @param c the character code provided by the user
   * @return true if it is in the set
   */
  bool isEscaped(wchar_t const c) const;

  /**
   * Flush all the blanks remaining in the current process
   * @param output stream to write blanks
   */
  void flushBlanks(FILE *output);

  bool isLastBlankTM;
public:
  FSTProcessor();
  ~FSTProcessor();

  void initGeneration();
  void load(FILE *input);

  void lsx(FILE *input, FILE *output);
};

#endif

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
   * Initial state of every token
   */
  State initial_state;

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
   * Begin of the transducer
   */
  Node root;

  /**
   * true if the position of input stream is out of a word
   */
  bool outOfWord;

  /**
   * Prints an error of input stream and exits
   */
  void streamError();

  /**
   * Reads a character that is defined in the set of escaped_chars
   * @param input the stream to read from
   * @return code of the character
   */
  wchar_t readEscaped(FILE *input);

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

  /**
   * Calculate the initial state of parsing
   */
  void calcInitial();

public:
  FSTProcessor();


  void initGeneration();
  void load(FILE *input);

  void lsx(FILE *input, FILE *output);


};

#endif

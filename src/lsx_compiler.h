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
#ifndef _MYCOMPILER_
#define _MYCOMPILER_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/regexp_compiler.h>
#include <lttoolbox/entry_token.h>
#include <lttoolbox/transducer.h>

#include <map>
#include <string>
#include <set>
#include <cstdint>
#include <libxml/xmlreader.h>

using namespace std;

/**
 * A compiler of dictionaries to letter transducers
 */
class Compiler
{
private:
    /**
     * The libxml2's XML reader
     */
    xmlTextReaderPtr reader = nullptr;

    /**
     * The alt value
     */
    UString alt;

    /**
     * The variant value (monodix)
     */
    UString variant;

    /**
     * The variant value (left side of bidix)
     */
    UString variant_left;

    /**
     * The variant value (right side of bidix)
     */
    UString variant_right;

    /**
     * The paradigm being compiled
     */
    UString current_paradigm;

    /**
     * The dictionary section being compiled
     */
    UString current_section;

    /**
     * The direction of the compilation, 'lr' (left-to-right) or 'rl'
     * (right-to-left)
     */
    UString direction;

    /**
     * List of characters to be considered alphabetic
     */
    UString letters;

    /**
     * Set verbose mode: warnings which may or may not be correct
     */
    bool verbose = false;

    /**
     * First element (of an entry)
     */
    bool first_element = false;

    /**
     * Identifier of all the symbols during the compilation
     */
    Alphabet alphabet;

    /**
     * Special symbols
     */
    int32_t any_tag = 0;
    int32_t any_char = 0;
    int32_t word_boundary = 0;

    /**
     * List of named transducers-paradigms
     */
    map<UString, Transducer> paradigms;

    /**
     * List of named dictionary sections
     */
    map<UString, Transducer> sections;

    /**
     * List of named prefix copy of a paradigm
     */
    map<UString, map<UString, int>> prefix_paradigms;

    /**
     * List of named suffix copy of a paradigm
     */
    map<UString, map<UString, int>> suffix_paradigms;

    /**
     * List of named endings of a suffix copy of a paradgim
     */
    map<UString, map<UString, int>> postsuffix_paradigms;

  template<typename... T>
  void error(const char* fmt, T... args) {
    UFILE* out = u_finit(stderr, NULL, NULL);
    u_fprintf(out, "Error on line %d: ",
              xmlTextReaderGetParserLineNumber(reader));
    u_fprintf(out, fmt, args...);
    u_fprintf(out, "\n");
    exit(EXIT_FAILURE);
  }

    /*
     static string range(char const a, char const b);
     string readAlphabet();
     */

    /**
     * Method to parse an XML Node
     */
    void procNode();


    /**
     * Parse the &lt;alphabet&gt; element
     */
    void procAlphabet();

    /**
     * Parse the &lt;sdef&lt; element
     */
    void procSDef();

    /**
     * Parse the &lt;pardef&gt; element
     */
    void procParDef();

    /**
     * Parse the &lt;e&gt; element
     */
    void procEntry();

    /**
     * Parse the &lt;j&gt; element
     */
    void procWb();

    /**
     * Parse the &lt;re&gt; element
     * @return a list of tokens from the dictionary's entry
     */
    EntryToken procRegexp();

    /**
     * Parse the &lt;section&gt; element
     */
    void procSection();

    /**
     * Gets an attribute value with their name and the current context
     * @param name the name of the attribute
     * @return the value of the attribute
     */
    UString attrib(UString const &name);

    /**
     * Construct symbol pairs by align left side of both parts and insert
     * them into a transducer
     * @param lp left part of the transduction
     * @param rp right part of the transduction
     * @param state the state from wich insert the new transduction
     * @param t the transducer
     * @return the last state of the inserted transduction
     */
    int matchTransduction(vector<int> const &lp, vector<int> const &rp,
                          int state, Transducer &t);
    /**
     * Parse the &lt;p&lt; element
     * @return a list of tokens from the dictionary's entry
     */
    EntryToken procTransduction();

    /**
     * Parse the &lt;i&lt; element
     * @return a list of tokens from the dictionary's entry
     */
    EntryToken procIdentity();

    /**
     * Parse the &lt;par&gt; element
     * @return a list of tokens from the dictionary's entry
     */
    EntryToken procPar();

    /**
     * Insert a list of tokens into the paradigm / section being processed
     * @param elements the list
     */
    void insertEntryTokens(vector<EntryToken> const &elements);

    /**
     * Skip all document #text nodes before "elem"
     * @param name the name of the node
     * @param elem the name of the expected node
     */
    void skip(UString &name, UString const &elem);

    /**
     * Skip all document #text nodes before "elem"
     * @param name the name of the node
     * @param elem the name of the expected node
     * @param open true for open element, false for closed
     */
    void skip(UString &name, UString const &elem, bool open);

    /**
     * Skip all blank #text nodes before "name"
     * @param name the name of the node
     */
    void skipBlanks(UString &name);


    void readString(vector<int> &result, UString const &name);

    /**
     * Force an element to be empty, and check for it
     * @param name the element
     */
    void requireEmptyError(UString const &name);

    /**
     * Force an attribute to be specified, amd check for it
     * @param value the value of the attribute
     * @param attrname the name of the attribute
     * @param elemname the parent of the attribute
     */

    void requireAttribute(UString const &value, UString const &attrname,
                          UString const &elemname);
    /**
     * True if all the elements in the current node are blanks
     * @return true if all are blanks
     */
    bool allBlanks();

public:

    /*
     * Constants to represent the element and the attributes of
     * dictionaries
     */
    static UString const COMPILER_DICTIONARY_ELEM;
    static UString const COMPILER_ALPHABET_ELEM;
    static UString const COMPILER_SDEFS_ELEM;
    static UString const COMPILER_SDEF_ELEM;
    static UString const COMPILER_N_ATTR;
    static UString const COMPILER_PARDEFS_ELEM;
    static UString const COMPILER_PARDEF_ELEM;
    static UString const COMPILER_PAR_ELEM;
    static UString const COMPILER_ENTRY_ELEM;
    static UString const COMPILER_RESTRICTION_ATTR;
    static UString const COMPILER_RESTRICTION_LR_VAL;
    static UString const COMPILER_RESTRICTION_RL_VAL;
    static UString const COMPILER_PAIR_ELEM;
    static UString const COMPILER_LEFT_ELEM;
    static UString const COMPILER_RIGHT_ELEM;
    static UString const COMPILER_S_ELEM;
    static UString const COMPILER_REGEXP_ELEM;
    static UString const COMPILER_SECTION_ELEM;
    static UString const COMPILER_ID_ATTR;
    static UString const COMPILER_TYPE_ATTR;
    static UString const COMPILER_IDENTITY_ELEM;
    static UString const COMPILER_JOIN_ELEM;
    static UString const COMPILER_BLANK_ELEM;
    static UString const COMPILER_POSTGENERATOR_ELEM;
    static UString const COMPILER_GROUP_ELEM;
    static UString const COMPILER_LEMMA_ATTR;
    static UString const COMPILER_IGNORE_ATTR;
    static UString const COMPILER_IGNORE_YES_VAL;
    static UString const COMPILER_ALT_ATTR;
    static UString const COMPILER_V_ATTR;
    static UString const COMPILER_VL_ATTR;
    static UString const COMPILER_VR_ATTR;

    static UString const COMPILER_ANYTAG_ELEM;
    static UString const COMPILER_ANYCHAR_ELEM;
    static UString const COMPILER_WB_ELEM;


    /**
     * Compile dictionary to letter transducers
     * @param fichero file
     * @param dir direction
     */
    void parse(string const &fichero, UString const &dir);

    //  auto getAlt();
    //  auto getInt();


    /**
     * Write the result of compilation
     * @param fd the stream where write the result
     */
    void write(FILE *fd);

    /**
     * Set verbose output
     */
    void setVerbose(bool verbosity = false);

};


#endif

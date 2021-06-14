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
#include <lsx_compiler.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/entry_token.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/string_utils.h>

#include <cstdlib>
#include <iostream>
#include <libxml/encoding.h>

using namespace std;

// Removed static globals copied from lttoolbox's compiler.cc. Same namespace, same mangling, bad result.

UString const Compiler::COMPILER_ANYTAG_ELEM        = "t"_u;
UString const Compiler::COMPILER_ANYCHAR_ELEM       = "w"_u;
UString const Compiler::COMPILER_WB_ELEM            = "j"_u;

void
Compiler::parse(string const &fichero, UString const &dir)
{
    direction = dir;
    reader = xmlReaderForFile(fichero.c_str(), NULL, 0);
    if(reader == NULL)
    {
        cerr << "Error: Cannot open '" << fichero.c_str() << "'." << endl;
        exit(EXIT_FAILURE);
    }

    alphabet.includeSymbol(Transducer::ANY_TAG_SYMBOL);
    alphabet.includeSymbol(Transducer::ANY_CHAR_SYMBOL);
    alphabet.includeSymbol(Transducer::LSX_BOUNDARY_SYMBOL);
    any_tag       = alphabet(Transducer::ANY_TAG_SYMBOL);
    any_char      = alphabet(Transducer::ANY_CHAR_SYMBOL);
    word_boundary = alphabet(Transducer::LSX_BOUNDARY_SYMBOL);

    int ret = xmlTextReaderRead(reader);
    while(ret == 1)
    {

        procNode();
        ret = xmlTextReaderRead(reader);
    }

    if(ret != 0)
    {
        cerr << "Error: Parse error at the end of input." << endl;
    }

    xmlFreeTextReader(reader);
    xmlCleanupParser();


    // Minimize transducers and ensure that all paths end with <$>
    int end_trans = alphabet(word_boundary, word_boundary);
    for (auto& it : sections) {
        it.second.minimize();
        // any paths which did not already end with <$> now will
        // having 2 finals isn't a problem because -separable only checks
        // for finals when it reads $, and you can't have 2 of those in a row
        for(auto fin : it.second.getFinals())
        {
          int end_state = it.second.insertSingleTransduction(end_trans, fin.first);
          it.second.setFinal(end_state);
        }
    }
}


void
Compiler::procAlphabet()
{
    int tipo=xmlTextReaderNodeType(reader);

    if(tipo != XML_READER_TYPE_END_ELEMENT)
    {
        int ret = xmlTextReaderRead(reader);
        if(ret == 1)
        {
            UString letters = XMLParseUtil::readValue(reader);
            bool espai = true;
            for(unsigned int i = 0; i < letters.length(); i++)
            {
                if(!isspace(letters.at(i)))
                {
                    espai = false;
                    break;
                }
            }
            if(espai == true)  // libxml2 returns '\n' for <alphabet></alphabet>, should be empty
            {
              letters.clear();
            }
        }
        else
        {
            cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
            cerr << "): Missing alphabet symbols." << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void
Compiler::procSDef()
{
  UString s;
  s += '<';
  s.append(attrib(COMPILER_N_ATTR));
  s += '>';
  alphabet.includeSymbol(s);
}

void
Compiler::procParDef()
{
    int tipo=xmlTextReaderNodeType(reader);

    if(tipo != XML_READER_TYPE_END_ELEMENT)
    {
        current_paradigm = attrib(COMPILER_N_ATTR);
    }
    else
    {
        if(!paradigms[current_paradigm].isEmpty())
        {
            paradigms[current_paradigm].minimize();
            paradigms[current_paradigm].joinFinals();
            current_paradigm.clear();
        }
    }
}

int
Compiler::matchTransduction(vector<int> const &pi, vector<int> const &pd, int estado, Transducer &t)
{
    vector<int>::const_iterator izqda, dcha, limizqda, limdcha;

    if(direction == COMPILER_RESTRICTION_LR_VAL)
    {
        izqda = pi.begin();
        dcha = pd.begin();
        limizqda = pi.end();
        limdcha = pd.end();
    }
    else
    {
        izqda = pd.begin();
        dcha = pi.begin();
        limizqda = pd.end();
        limdcha = pi.end();
    }


    if(pi.size() == 0 && pd.size() == 0)
    {
        estado = t.insertNewSingleTransduction(alphabet(0, 0), estado);
    }
    else
    {
        while(true)
        {
            int etiqueta;


            if(izqda == limizqda && dcha == limdcha)
            {
                break;
            }
            else if(izqda == limizqda)
            {
                etiqueta = alphabet(0, *dcha);
                dcha++;
            }
            else if(dcha == limdcha)
            {
                etiqueta = alphabet(*izqda, 0);
                izqda++;
            }
            else
            {
                etiqueta = alphabet(*izqda, *dcha);
                izqda++;
                dcha++;
            }

            if(etiqueta == alphabet(0, any_tag) ||
               etiqueta == alphabet(0, any_char)
              )
            {
              // rl compilation of a badly written rule
              // having an epsilon with wildcard output will produce
              // garbage output -- see https://github.com/apertium/apertium-separable/issues/8
              cerr << "Warning: Cannot insert <t/> from empty input. Ignoring. (You probably want to specify exact tags when deleting a word.)" << endl;
              continue;
            }

            int nuevo_estado = t.insertSingleTransduction(etiqueta, estado);
            if(etiqueta == alphabet(any_tag, any_tag)
               || etiqueta == alphabet(any_char, any_char)
               || etiqueta == alphabet(any_tag, 0)
               || etiqueta == alphabet(any_char, 0)
              )
            {
                t.linkStates(nuevo_estado, estado, 0);
            }
            estado = nuevo_estado;
        }
    }

    return estado;
}


void
Compiler::requireEmptyError(UString const &name)
{
    if(!xmlTextReaderIsEmptyElement(reader))
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Non-empty element '<" << name << ">' should be empty." << endl;
        exit(EXIT_FAILURE);
    }
}

bool
Compiler::allBlanks()
{
  vector<int32_t> text;
  XMLParseUtil::readValueInto32(reader, text);
  for (auto& it : text) {
    if (!u_isspace(it)) {
      return false;
    }
  }
  return true;
}

void
Compiler::readString(vector<int> &result, UString const &name)
{
    if(name == "#text"_u)
    {
      XMLParseUtil::readValueInto32(reader, result);
    }
    else if(name == COMPILER_BLANK_ELEM)
    {
        requireEmptyError(name);
        result.push_back(static_cast<int>(' '));
    }
    else if(name == COMPILER_POSTGENERATOR_ELEM)
    {
        requireEmptyError(name);
        result.push_back(static_cast<int>('~'));
    }
    else if(name == COMPILER_GROUP_ELEM)
    {
        int tipo=xmlTextReaderNodeType(reader);
        if(tipo != XML_READER_TYPE_END_ELEMENT)
        {
            result.push_back(static_cast<int>('#'));
        }
    }
    else if(name == COMPILER_S_ELEM)
    {
        requireEmptyError(name);
        UString symbol;
        symbol += '<';
        symbol.append(attrib(COMPILER_N_ATTR));
        symbol += '>';

        if(!alphabet.isSymbolDefined(symbol))
        {
            cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
            cerr << "): Undefined symbol '" << symbol << "'." << endl;
            exit(EXIT_FAILURE);
        }
        result.push_back(alphabet(symbol));
    }
    else if(name == COMPILER_ANYTAG_ELEM)
    {
        result.push_back(any_tag);
    }
    else if(name == COMPILER_ANYCHAR_ELEM)
    {
        result.push_back(any_char);
    }
    else if(name == COMPILER_WB_ELEM)
    {
        requireEmptyError(name);
        result.push_back(word_boundary);
    }

    else
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Invalid specification of element '<" << name;
        cerr << ">' in this context." << endl;
        exit(EXIT_FAILURE);
    }
}

void
Compiler::skipBlanks(UString &name)
{
    while(name == "#text"_u || name == "#comment"_u)
    {
        if(name != "#comment"_u)
        {
            if(!allBlanks())
            {
                cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
                cerr << "): Invalid construction." << endl;
                exit(EXIT_FAILURE);
            }
        }

        xmlTextReaderRead(reader);
        name = XMLParseUtil::readName(reader);
    }
}

void
Compiler::skip(UString &name, UString const &elem)
{
    skip(name, elem, true);
}

void
Compiler::skip(UString &name, UString const &elem, bool open)
{
    xmlTextReaderRead(reader);
    name = XMLParseUtil::readName(reader);
    UString slash;

    if(!open)
    {
        slash = "/"_u;
    }

    while(name == "#text"_u || name == "#comment"_u)
    {
        if(name != "#comment"_u)
        {
            if(!allBlanks())
            {
                cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
                cerr << "): Invalid construction." << endl;
                exit(EXIT_FAILURE);
            }
        }
        xmlTextReaderRead(reader);
        name = XMLParseUtil::readName(reader);
    }

    if(name != elem)
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Expected '<" << slash << elem << ">'." << endl;
        exit(EXIT_FAILURE);
    }
}

EntryToken
Compiler::procIdentity()
{
    vector<int> both_sides;

    if(!xmlTextReaderIsEmptyElement(reader))
    {
        UString name;

        while(true)
        {
            xmlTextReaderRead(reader);
            name = XMLParseUtil::readName(reader);
            if(name == COMPILER_IDENTITY_ELEM)
            {
                break;
            }
            readString(both_sides, name);
        }
    }

    if(verbose && first_element && (both_sides.front() == (int)' '))
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Entry begins with space." << endl;
    }
    first_element = false;
    EntryToken e;
    e.setSingleTransduction(both_sides, both_sides);
    return e;
}

EntryToken
Compiler::procTransduction()
{
    vector<int> lhs, rhs;
    UString name;

    skip(name, COMPILER_LEFT_ELEM);

    if(!xmlTextReaderIsEmptyElement(reader))
    {
      name.clear();
        while(true)
        {
            xmlTextReaderRead(reader);
            name = XMLParseUtil::readName(reader);
            if(name == COMPILER_LEFT_ELEM)
            {
                break;
            }
            readString(lhs, name);
        }
    }

    if(verbose && first_element && (lhs.front() == (int)' '))
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Entry begins with space." << endl;
    }
    first_element = false;

    skip(name, COMPILER_RIGHT_ELEM);

    if(!xmlTextReaderIsEmptyElement(reader))
    {
      name.clear();
        while(true)
        {
            xmlTextReaderRead(reader);
            name = XMLParseUtil::readName(reader);
            if(name == COMPILER_RIGHT_ELEM)
            {
                break;
            }
            readString(rhs, name);
        }
    }

    skip(name, COMPILER_PAIR_ELEM, false);

    EntryToken e;
    e.setSingleTransduction(lhs, rhs);

    return e;
}

UString
Compiler::attrib(UString const &name)
{
    return XMLParseUtil::attrib(reader, name);
}

EntryToken
Compiler::procPar()
{
    EntryToken e;
    UString nomparadigma = attrib(COMPILER_N_ATTR);
    first_element = false;

    if(!current_paradigm.empty() && nomparadigma == current_paradigm)
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Paradigm refers to itself '" << nomparadigma << "'." <<endl;
        exit(EXIT_FAILURE);
    }

    if(paradigms.find(nomparadigma) == paradigms.end())
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Undefined paradigm '" << nomparadigma << "'." << endl;
        exit(EXIT_FAILURE);
    }
    e.setParadigm(nomparadigma);
    return e;
}

void
Compiler::insertEntryTokens(vector<EntryToken> const &elements)
{
  if(!current_paradigm.empty()) {
        // compilation of paradigms
        Transducer &t = paradigms[current_paradigm];
        int e = t.getInitial();

        for(unsigned int i = 0, limit = elements.size(); i < limit; i++)
        {
            if(elements[i].isParadigm())
            {
                e = t.insertTransducer(e, paradigms[elements[i].paradigmName()]);
            }
            else if(elements[i].isSingleTransduction())
            {
                e = matchTransduction(elements[i].left(), elements[i].right(), e, t);
            }
            else if(elements[i].isRegexp())
            {
                RegexpCompiler analyzer;
                analyzer.initialize(&alphabet);
                analyzer.compile(elements[i].regExp());
                e = t.insertTransducer(e, analyzer.getTransducer(), alphabet(0,0));
            }
            else
            {
                cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
                cerr << "): Invalid entry token." << endl;
                exit(EXIT_FAILURE);
            }
        }
        t.setFinal(e);
    }
    else
    {
        // compilaci���n de dictionary

        Transducer &t = sections[current_section];
        int e = t.getInitial();

        for(unsigned int i = 0, limit = elements.size(); i < limit; i++)
        {
            if(elements[i].isParadigm())
            {
                if(i == elements.size()-1)
                {
                    // paradigma sufijo
                    if(suffix_paradigms[current_section].find(elements[i].paradigmName()) != suffix_paradigms[current_section].end())
                    {
                        t.linkStates(e, suffix_paradigms[current_section][elements[i].paradigmName()], 0);
                        e = postsuffix_paradigms[current_section][elements[i].paradigmName()];
                    }
                    else
                    {
                        e = t.insertNewSingleTransduction(alphabet(0, 0), e);
                        suffix_paradigms[current_section][elements[i].paradigmName()] = e;
                        e = t.insertTransducer(e, paradigms[elements[i].paradigmName()]);
                        postsuffix_paradigms[current_section][elements[i].paradigmName()] = e;
                    }
                }
                else if(i == 0)
                {
                    // paradigma prefijo
                    if(prefix_paradigms[current_section].find(elements[i].paradigmName()) != prefix_paradigms[current_section].end())
                    {
                        e = prefix_paradigms[current_section][elements[i].paradigmName()];
                    }
                    else
                    {
                        e = t.insertTransducer(e, paradigms[elements[i].paradigmName()]);
                        prefix_paradigms[current_section][elements[i].paradigmName()] = e;
                    }
                }
                else
                {
                    // paradigma intermedio
                    e = t.insertTransducer(e, paradigms[elements[i].paradigmName()]);
                }
            }
            else if(elements[i].isRegexp())
            {
                RegexpCompiler analyzer;
                analyzer.initialize(&alphabet);
                analyzer.compile(elements[i].regExp());
                e = t.insertTransducer(e, analyzer.getTransducer(), alphabet(0,0));
            }
            else
            {
                e = matchTransduction(elements[i].left(), elements[i].right(), e, t);
            }
        }
        t.setFinal(e);
    }
}


void
Compiler::requireAttribute(UString const &value, UString const &attrname,
                           UString const &elemname)
{
  if(value.empty()) {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): '<" << elemname;
        cerr << "' element must specify non-void '";
        cerr << attrname << "' attribute." << endl;
        exit(EXIT_FAILURE);
    }
}


void
Compiler::procSection()
{
    int tipo=xmlTextReaderNodeType(reader);

    if(tipo != XML_READER_TYPE_END_ELEMENT)
    {
        UString const &id = attrib(COMPILER_ID_ATTR);
        UString const &type = attrib(COMPILER_TYPE_ATTR);
        requireAttribute(id, COMPILER_ID_ATTR, COMPILER_SECTION_ELEM);
        requireAttribute(type, COMPILER_TYPE_ATTR, COMPILER_SECTION_ELEM);

        current_section = id;
        current_section += '@';
        current_section.append(type);
    }
    else
    {
      current_section.clear();
    }
}

void
Compiler::procEntry()
{
    UString atributo=this->attrib(COMPILER_RESTRICTION_ATTR);
    UString ignore = this->attrib(COMPILER_IGNORE_ATTR);
    UString altval = this->attrib(COMPILER_ALT_ATTR);
    UString varval = this->attrib(COMPILER_V_ATTR);
    UString varl   = this->attrib(COMPILER_VL_ATTR);
    UString varr   = this->attrib(COMPILER_VR_ATTR);

    // if entry is masked by a restriction of direction or an ignore mark
    if((!atributo.empty() && atributo != direction)
       || ignore == COMPILER_IGNORE_YES_VAL
       || (!altval.empty() && altval != alt)
       || (direction == COMPILER_RESTRICTION_RL_VAL && !varval.empty() && varval != variant)
       || (direction == COMPILER_RESTRICTION_RL_VAL && !varl.empty() && varl != variant_left)
       || (direction == COMPILER_RESTRICTION_LR_VAL && !varr.empty() && varr != variant_right))
    {
        // parse to the end of the entry
        UString name;

        while(name != COMPILER_ENTRY_ELEM)
        {
            xmlTextReaderRead(reader);
            name = XMLParseUtil::readName(reader);
        }

        return;
    }

    vector<EntryToken> elements;

    while(true)
    {
        int ret = xmlTextReaderRead(reader);
        if(ret != 1)
        {
            cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
            cerr << "): Parse error." << endl;
            exit(EXIT_FAILURE);
        }
        UString name = XMLParseUtil::readName(reader);
        skipBlanks(name);

        if(current_paradigm.empty() && verbose)
        {
            first_element = true;
        }

        int tipo = xmlTextReaderNodeType(reader);
        if(name == COMPILER_PAIR_ELEM)
        {
            elements.push_back(procTransduction());
        }
        else if(name == COMPILER_IDENTITY_ELEM)
        {
            elements.push_back(procIdentity());
        }
        else if(name == COMPILER_REGEXP_ELEM)
        {
            elements.push_back(procRegexp());
        }
        else if(name == COMPILER_PAR_ELEM)
        {
            elements.push_back(procPar());

            // detecci���n del uso de paradigmas no definidos

            UString const &p = elements.rbegin()->paradigmName();

            if(paradigms.find(p) == paradigms.end())
            {
                cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
                cerr << "): Undefined paradigm '" << p << "'." <<endl;
                exit(EXIT_FAILURE);
            }
            // descartar entradas con paradigms vac���os (por las direciones,
            // normalmente
            if(paradigms[p].isEmpty())
            {
                while(name != COMPILER_ENTRY_ELEM || tipo != XML_READER_TYPE_END_ELEMENT)
                {
                    xmlTextReaderRead(reader);
                    name = XMLParseUtil::readName(reader);
                    tipo = xmlTextReaderNodeType(reader);
                }
                return;
            }
        }
        else if(name == COMPILER_ENTRY_ELEM && tipo == XML_READER_TYPE_END_ELEMENT)
        {
            /* INSERTING FINAL <$> HERE */
            // vector<int> wb;
            // wb.push_back(word_boundary);
            // EntryToken e;
            // e.setSingleTransduction(wb, wb);
            // elements.push_back(e);

            // insertar elements into letter transducer
            insertEntryTokens(elements);
            return;
        }
        else
        {
            cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
            cerr << "): Invalid inclusion of '<" << name << ">' into '<" << COMPILER_ENTRY_ELEM;
            cerr << ">'." << endl;
            exit(EXIT_FAILURE);
        }

    }
}

void
Compiler::procNode()
{
    UString nombre = XMLParseUtil::readName(reader);

    // HACER: optimizar el orden de ejecución de esta ristra de "ifs"

    if(nombre == "#text"_u)
    {
        /* ignorar */
    }
    else if(nombre == COMPILER_DICTIONARY_ELEM)
    {
        /* ignorar */
    }
    else if(nombre == COMPILER_ALPHABET_ELEM)
    {
        procAlphabet();
    }
    else if(nombre == COMPILER_SDEFS_ELEM)
    {
        /* ignorar */
    }
    else if(nombre == COMPILER_SDEF_ELEM)
    {
        procSDef();
    }
    else if(nombre == COMPILER_PARDEFS_ELEM)
    {
        /* ignorar */
    }
    else if(nombre == COMPILER_PARDEF_ELEM)
    {
        procParDef();
    }
    else if(nombre == COMPILER_ENTRY_ELEM)
    {
        procEntry();
    }
    else if(nombre == COMPILER_SECTION_ELEM)
    {
        procSection();
    }
    else if(nombre == "#comment"_u)
    {
        /* ignorar */
    }
    else
    {
        cerr << "Error (" << xmlTextReaderGetParserLineNumber(reader);
        cerr << "): Invalid node '<" << nombre << ">'." << endl;
        exit(EXIT_FAILURE);
    }
}

EntryToken
Compiler::procRegexp()
{
    EntryToken et;
    xmlTextReaderRead(reader);
    UString re = XMLParseUtil::readValue(reader);
    et.setRegexp(re);
    xmlTextReaderRead(reader);
    return et;
}

void
Compiler::write(FILE *output)
{
    // letters
    Compression::string_write(letters, output);

    // symbols
    alphabet.write(output);

    // transducers
    Compression::multibyte_write(sections.size(), output);

    for (auto& it : sections) {
        cout << it.first << " " << it.second.size();
        cout << " " << it.second.numberOfTransitions() << endl;
        Compression::string_write(it.first, output);
        it.second.write(output);
    }
}

void
Compiler::setVerbose(bool verbosity)
{
    verbose = verbosity;
}

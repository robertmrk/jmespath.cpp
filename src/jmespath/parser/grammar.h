/****************************************************************************
**
** Author: Róbert Márki <gsmiko@gmail.com>
** Copyright (c) 2016 Róbert Márki
**
** This file is part of the jmespath.cpp project which is distributed under
** the MIT License (MIT).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#ifndef GRAMMAR_H
#define GRAMMAR_H
#include "jmespath/detail/types.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

/**
 * @namespace jmespath::parser
 * @brief Classes required for parsing JMESPath expressions
 */
namespace jmespath { namespace parser {

namespace qi = boost::spirit::qi;
namespace encoding = qi::standard_wide;
namespace phx = boost::phoenix;
using namespace detail;

/**
 * @brief The Grammar class contains the PEG rule definition based
 * on the EBNF specifications of JMESPath.
 * @tparam Iterator String iterator type
 * @tparam Skipper Character skipper parser type
 * @sa http://jmespath.org/specification.html#grammar
 */
template <typename Iterator, typename Skipper = encoding::space_type>
class Grammar : public qi::grammar<Iterator, String(), Skipper>
{
    public:
    /**
     * @brief Constructs a Grammar object
     */
    Grammar() : Grammar::base_type(m_identifierRule)
    {
        using encoding::char_;
        using qi::lit;
        using qi::lexeme;
        using qi::int_parser;

        m_identifierRule = m_unquotedStringRule | m_quotedStringRule;
        m_unquotedStringRule
                = lexeme[ ((char_(U'\x41', U'\x5A')
                            | char_(U'\x61', U'\x7A')
                            | char_(U'\x5F'))[phx::bind(&Grammar::appendUtf8,
                                                        this,
                                                        qi::_val,
                                                        qi::_1)]
                >> *(char_(U'\x30', U'\x39')
                     | char_(U'\x41', U'\x5A')
                     | char_(U'\x5F')
                     | char_(U'\x61', U'\x7A'))[phx::bind(&Grammar::appendUtf8,
                                                          this,
                                                          qi::_val,
                                                          qi::_1)]) ];
        m_quotedStringRule
                = lexeme[ m_quoteRule
                >> +(m_unescapedCharRule
                     | m_escapedCharRule)[phx::bind(&Grammar::appendUtf8,
                                                  this,
                                                  qi::_val,
                                                  qi::_1)]
                >> m_quoteRule ];
        m_unescapedCharRule = char_(U'\x20', U'\x21')
                | char_(U'\x23', U'\x5B')
                | char_(U'\x5D', U'\U0010FFFF');
        m_quoteRule = lit('\"');
        m_escapeRule = lit('\\');
        m_escapedCharRule = lexeme[ m_escapeRule >> (char_(U'\"')
                                                 | char_(U'\\')
                                                 | char_(U'/')
                                                 | m_controlCharacterSymbols
                                                 | m_surrogatePairCharacterRule
                                                 | m_unicodeCharRule) ];
        m_surrogatePairCharacterRule
                = lexeme[ (m_unicodeCharRule >> m_escapeRule >> m_unicodeCharRule)
                [qi::_pass = (qi::_1 >= 0xD800 && qi::_1 <= 0xDBFF),
                qi::_val = phx::bind(&Grammar::parseSurrogatePair,
                                     this,
                                     qi::_1,
                                     qi::_2)] ];
        m_unicodeCharRule = lexeme[ lit('u')
                >> int_parser<UnicodeChar, 16, 4, 4>() ];
        m_controlCharacterSymbols.add
                (U"b", U'\x08')     // backspace
                (U"f", U'\x0C')     // form feed
                (U"n", U'\x0A')     // line feed
                (U"r", U'\x0D')     // carriage return
                (U"t", U'\x09');    // tab
    }

    private:
    qi::rule<Iterator, String(), Skipper>   m_identifierRule;
    qi::rule<Iterator, String()>            m_quotedStringRule;
    qi::rule<Iterator, String()>            m_unquotedStringRule;
    qi::rule<Iterator, UnicodeChar()>       m_unescapedCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_escapedCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_unicodeCharRule;
    qi::rule<Iterator, UnicodeChar()>       m_surrogatePairCharacterRule;
    qi::rule<Iterator>                      m_quoteRule;
    qi::rule<Iterator>                      m_escapeRule;
    qi::symbols<UnicodeChar, UnicodeChar>   m_controlCharacterSymbols;

    /**
     * @brief Appends the \a utf32Char character to the \a utf8String encoded in
     * UTF-8.
     * @param utf8String The string where the encoded value of the \a utf32Char
     * will be appended.
     * @param utf32Char The input character encoded in UTF-32
     */
    void appendUtf8(String& utf8String, UnicodeChar utf32Char) const
    {
        auto outIt = std::back_inserter(utf8String);
        boost::utf8_output_iterator<decltype(outIt)> utf8OutIt(outIt);
        *utf8OutIt++ = utf32Char;
    }
    /**
     * @brief Parses a surrogate pair character
     * @param highSurrogate High surrogate
     * @param lowSurrogate Low surrogate
     * @return The result of @a highSurrogate and @a lowSurrogate combined
     * into a single codepoint
     */
    UnicodeChar parseSurrogatePair(UnicodeChar const& highSurrogate, UnicodeChar const& lowSurrogate)
    {
        UnicodeChar unicodeChar = 0x10000;
        unicodeChar += (highSurrogate & 0x03FF) << 10;
        unicodeChar += (lowSurrogate & 0x03FF);
        return unicodeChar;
    }
};
}} // jmespath::parser
#endif // GRAMMAR_H

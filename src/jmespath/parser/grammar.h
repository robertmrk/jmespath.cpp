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
#include "jmespath/ast/identifiernode.h"
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
class Grammar : public qi::grammar<Iterator, ast::IdentifierNode(), Skipper>
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
        using qi::_val;
        using qi::_1;
        using qi::_2;
        using qi::_pass;

        // match unquoted or quoted strings
        m_identifierRule = m_unquotedStringRule | m_quotedStringRule;

        // match a single character in the range of 0x41-0x5A or 0x61-0x7A
        // or 0x5F (A-Za-z_) followed by zero or more characters in the range of
        // 0x30-0x39 (0-9) or 0x41-0x5A (A-Z) or 0x5F (_) or 0x61-0x7A (a-z)
        // and append them to the rule's string attribute encoded as UTF-8
        m_unquotedStringRule
            = lexeme[ ((char_(U'\x41', U'\x5A')
                        | char_(U'\x61', U'\x7A')
                        | char_(U'\x5F'))[phx::bind(&Grammar::appendUtf8,
                                                    this,
                                                    _val,
                                                    _1)]
            >> *(char_(U'\x30', U'\x39')
                 | char_(U'\x41', U'\x5A')
                 | char_(U'\x5F')
                 | char_(U'\x61', U'\x7A'))[phx::bind(&Grammar::appendUtf8,
                                                      this,
                                                      _val,
                                                      _1)]) ];

        // match unescaped or escaped characters enclosed in quotes one or more
        // times and append them to the rule's string attribute encoded as UTF-8
        m_quotedStringRule
            = lexeme[ m_quoteRule
            >> +(m_unescapedCharRule
                 | m_escapedCharRule)[phx::bind(&Grammar::appendUtf8,
                                                this,
                                                _val,
                                                _1)]
            >> m_quoteRule ];

        // match characters in the range of 0x20-0x21 or 0x23-0x5B or
        // 0x5D-0x10FFFF
        m_unescapedCharRule = char_(U'\x20', U'\x21')
            | char_(U'\x23', U'\x5B')
            | char_(U'\x5D', U'\U0010FFFF');

        // match quotation mark literal
        m_quoteRule = lit('\"');

        // match backslash literal
        m_escapeRule = lit('\\');

        // match an escape character followed by quotation mark or backslash or
        // slash or control character or surrogate pair or a single unicode
        // escape
        m_escapedCharRule = lexeme[ m_escapeRule
            >> (char_(U'\"')
                | char_(U'\\')
                | char_(U'/')
                | m_controlCharacterSymbols
                | m_surrogatePairCharacterRule
                | m_unicodeCharRule) ];

        // match a pair of unicode character escapes separated by an escape
        // symbol if the first character's value is between 0xD800-0xDBFF
        // and convert them into a single codepoint
        m_surrogatePairCharacterRule
            = lexeme[ (m_unicodeCharRule >> m_escapeRule >> m_unicodeCharRule)
                [_pass = (_1 >= 0xD800 && _1 <= 0xDBFF),
                _val = phx::bind(&Grammar::parseSurrogatePair,
                                this,
                                _1,
                                _2)] ];

        // match a unicode character escape and convert it into a
        // single codepoint
        m_unicodeCharRule = lexeme[ lit('u')
            >> int_parser<UnicodeChar, 16, 4, 4>() ];

        // convert symbols into control characters
        m_controlCharacterSymbols.add
        (U"b", U'\x08')     // backspace
        (U"f", U'\x0C')     // form feed
        (U"n", U'\x0A')     // line feed
        (U"r", U'\x0D')     // carriage return
        (U"t", U'\x09');    // tab
    }

private:
    qi::rule<Iterator, ast::IdentifierNode(), Skipper>   m_identifierRule;
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
    UnicodeChar parseSurrogatePair(UnicodeChar const& highSurrogate,
                                   UnicodeChar const& lowSurrogate)
    {
        UnicodeChar unicodeChar = 0x10000;
        unicodeChar += (highSurrogate & 0x03FF) << 10;
        unicodeChar += (lowSurrogate & 0x03FF);
        return unicodeChar;
    }
};
}} // namespace jmespath::parser
#endif // GRAMMAR_H
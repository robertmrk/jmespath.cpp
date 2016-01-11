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

/**
 * @namespace jmespath::parser
 * @brief Classes required for parsing JMESPath expressions
 */
namespace jmespath { namespace parser {

namespace qi = boost::spirit::qi;
using namespace detail;

/**
 * @brief The Grammar class contains the PEG rule definition based
 * on the EBNF specifications of JMESPath.
 * @tparam Iterator String iterator type
 * @tparam Skipper Character skipper parser type
 * @sa http://jmespath.org/specification.html#grammar
 */
template <typename Iterator, typename Skipper = qi::standard_wide::space_type>
struct Grammar : qi::grammar<Iterator, String(), Skipper>
{
    /**
     * @brief Constructs a Grammar object
     */
    Grammar() : Grammar::base_type(identifierRule)
    {
        using qi::ascii::char_;
        using qi::lit;
        using qi::lexeme;
        using qi::int_parser;

        identifierRule = unquotedStringRule | quotedStringRule;
        unquotedStringRule = lexeme[ (char_('\x41', '\x5A')
                                      | char_('\x61', '\x7A')
                                      | char_('\x5F'))
                >> *(char_('\x30', '\x39')
                     | char_('\x41', '\x5A')
                     | char_('\x5F')
                     | char_('\x61', '\x7A')) ];
        quotedStringRule = lexeme[ quoteRule >> +(unescapedCharRule
                                                  | escapedCharRule)
                                             >> quoteRule ];
        unescapedCharRule = char_('\x20', '\x21')
                | char_('\x23', '\x5B')
                | char_('\x5D', '\x7E');
        quoteRule = lit('\"');
        escapeRule = lit('\\');
        escapedCharRule = lexeme[ escapeRule >> (char_('\"')
                                                  | char_('\\')
                                                  | char_('/')
                                                  | controlCharacterSymbols
                                                  | unicodeCharRule) ];
        unicodeCharRule = lexeme[ lit('u')
                >> int_parser<Char, 16, 4, 4>() ];
        controlCharacterSymbols.add
                ("b", '\x08')     // backspace
                ("f", '\x0C')     // form feed
                ("n", '\x0A')     // line feed
                ("r", '\x0D')     // carriage return
                ("t", '\x09');    // tab
    }

    qi::rule<Iterator, String(), Skipper>   identifierRule;
    qi::rule<Iterator, String()>            quotedStringRule;
    qi::rule<Iterator, String()>            unquotedStringRule;
    qi::rule<Iterator, Char()>              unescapedCharRule;
    qi::rule<Iterator, Char()>              escapedCharRule;
    qi::rule<Iterator, Char()>              unicodeCharRule;
    qi::rule<Iterator>                      quoteRule;
    qi::rule<Iterator>                      escapeRule;
    qi::symbols<Char, Char>                 controlCharacterSymbols;
};

}} // jmespath::parser
#endif // GRAMMAR_H

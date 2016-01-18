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
#include "fakeit.hpp"
#include "jmespath/parser/grammar.h"
#include "jmespath/detail/types.h"
#include <boost/spirit/include/qi.hpp>

using namespace jmespath::parser;
using namespace jmespath::detail;

template <typename GrammarT>
typename GrammarT::start_type::attr_type
parseExpression(const GrammarT& grammar, const String& expression)
{
    namespace qi = boost::spirit::qi;
    namespace encoding = qi::standard_wide;

    typename GrammarT::start_type::attr_type result;
    UnicodeIteratorAdaptor it(expression.cbegin());
    UnicodeIteratorAdaptor endIt(expression.cend());

    qi::phrase_parse(it, endIt,
                     grammar, encoding::space,
                     result);
    return result;
}

TEST_CASE("Grammar")
{
    using namespace jmespath::parser;
    namespace qi = boost::spirit::qi;
    namespace ast = jmespath::ast;

    Grammar<UnicodeIteratorAdaptor> grammar;

    SECTION("can be used to parse")
    {
        SECTION("unquoted string")
        {
            REQUIRE(parseExpression(grammar, "identifierName")
                    == ast::IdentifierNode{"identifierName"});
        }

        SECTION("quoted string")
        {
            REQUIRE(parseExpression(grammar,
                                    "\"identifier with space\"")
                    == ast::IdentifierNode{"identifier with space"});
        }

        SECTION("string with escaped characters")
        {
            REQUIRE(parseExpression(grammar, "\"\\\\\\\"\\/\"")
                    == ast::IdentifierNode{"\\\"/"});
        }

        SECTION("string with escaped symbols")
        {
            REQUIRE(parseExpression(grammar, "\"\\t\\n\\b\"")
                    == ast::IdentifierNode{"\t\n\b"});
        }

        SECTION("string with unicode escapes")
        {
            REQUIRE(parseExpression(grammar, "\"\\u20AC\"")
                    == ast::IdentifierNode{"\xE2\x82\xAC"});
        }

        SECTION("string with encoded unicode characters")
        {
            REQUIRE(parseExpression(grammar, "\"\xE2\x82\xAC\"")
                    == ast::IdentifierNode{"\xE2\x82\xAC"});
        }

        SECTION("string with surrogate pair unicode escapes")
        {
            REQUIRE(parseExpression(grammar, "\"\\uD834\\uDD1E\"")
                    == ast::IdentifierNode{u8"\U0001D11E"});
        }
    }
}

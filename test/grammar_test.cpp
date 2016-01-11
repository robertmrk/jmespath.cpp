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
#include "catch.hpp"
#include "jmespath/parser/grammar.h"
#include <boost/spirit/include/qi.hpp>

using namespace jmespath::parser;

template <typename GrammarT>
typename GrammarT::start_type::attr_type
parseExpression(const GrammarT& grammar, const String& expression)
{
    namespace qi = boost::spirit::qi;

    typename GrammarT::start_type::attr_type result;
    auto it = std::cbegin(expression);
    auto endIt = std::cend(expression);

    qi::phrase_parse(it, endIt,
                     grammar, qi::standard_wide::space,
                     result);
    return result;
}

TEST_CASE("Grammar")
{
    using namespace jmespath::parser;
    namespace qi = boost::spirit::qi;

    Grammar<String::const_iterator> grammar;

    SECTION("can be used to parse unquoted string")
    {
        REQUIRE(parseExpression(grammar, "identifierName") == "identifierName");
    }

    SECTION("can be used to parse quoted string")
    {
        REQUIRE(parseExpression(grammar, "\"identifier with space\"")
                == "identifier with space");
    }

    SECTION("can be used to parse string with escaped characters")
    {
        REQUIRE(parseExpression(grammar, "\"\\\\\\\"\\/\"") == "\\\"/");
    }

    SECTION("can be used to parse string with escaped symbols")
    {
        REQUIRE(parseExpression(grammar, "\"\\t\\n\\b\"") == "\t\n\b");
    }

    SECTION("can be used to parse string with unicode escapes")
    {
        REQUIRE(parseExpression(grammar, "\"\\u0041\"") == "A");
    }
}

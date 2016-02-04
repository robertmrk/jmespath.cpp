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
    namespace encoding = qi::unicode;

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
            REQUIRE(parseExpression(grammar, "identifierName").expression
                    == ast::IdentifierNode{"identifierName"});
        }

        SECTION("quoted string")
        {
            REQUIRE(parseExpression(grammar,
                                    "\"identifier with space\"").expression
                    == ast::IdentifierNode{"identifier with space"});
        }

        SECTION("string with escaped characters")
        {
            REQUIRE(parseExpression(grammar, "\"\\\\\\\"\\/\"").expression
                    == ast::IdentifierNode{"\\\"/"});
        }

        SECTION("string with escaped symbols")
        {
            REQUIRE(parseExpression(grammar, "\"\\t\\n\\b\"").expression
                    == ast::IdentifierNode{"\t\n\b"});
        }

        SECTION("string with unicode escapes")
        {
            REQUIRE(parseExpression(grammar, "\"\\u20AC\"").expression
                    == ast::IdentifierNode{"\xE2\x82\xAC"});
        }

        SECTION("string with encoded unicode characters")
        {
            REQUIRE(parseExpression(grammar, u8"\"\U00103C02\"").expression
                    == ast::IdentifierNode{u8"\U00103C02"});
        }

        SECTION("string with surrogate pair unicode escapes")
        {
            REQUIRE(parseExpression(grammar, "\"\\uD834\\uDD1E\"").expression
                    == ast::IdentifierNode{u8"\U0001D11E"});
        }

        SECTION("raw string")
        {
            REQUIRE(parseExpression(grammar, "'[ba\\'z]'").expression
                    == ast::RawStringNode{"[ba'z]"});
        }

        SECTION("literals")
        {
            REQUIRE(parseExpression(grammar, "`\"foo\\`bar\"`").expression
                    == ast::LiteralNode{"\"foo`bar\""});
            REQUIRE(parseExpression(grammar, "`[1, 2]`").expression
                    == ast::LiteralNode{"[1, 2]"});
        }

        SECTION("subexpression")
        {
            auto expectedResult = ast::SubexpressionNode{
                    ast::ExpressionNode{
                        ast::IdentifierNode{"id1"}},
                    ast::ExpressionNode{
                        ast::IdentifierNode{"id2"}}};

            REQUIRE(parseExpression(grammar, "\"id1\".\"id2\"").expression
                    == expectedResult);
        }

        SECTION("recursive subexpression")
        {
            auto expectedResult = ast::SubexpressionNode{
                ast::ExpressionNode{
                    ast::SubexpressionNode{
                    ast::ExpressionNode{
                        ast::SubexpressionNode{
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id1"}},
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id2"}}}},
                    ast::ExpressionNode{
                        ast::IdentifierNode{"id3"}}}},
                ast::ExpressionNode{
                    ast::IdentifierNode{"id4"}}};
            String expression{"\"id1\".\"id2\".\"id3\".\"id4\""};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("index expression following an expression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}},
                ast::BracketSpecifierNode{ast::ArrayItemNode{3}}};
            String expression{"\"id\"[3]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("recursive index expression following an expression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{
                    ast::IndexExpressionNode{
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id"}},
                    ast::BracketSpecifierNode{
                        ast::ArrayItemNode{2}}}},
                ast::BracketSpecifierNode{
                    ast::ArrayItemNode{3}}};
            String expression{"\"id\"[2][3]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("standalon index expression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{},
                ast::BracketSpecifierNode{
                    ast::ArrayItemNode{3}}};
            String expression{"[3]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("recursive index expression following a standalon index "
                "expresion")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{
                    ast::IndexExpressionNode{
                        ast::ExpressionNode{
                            ast::IndexExpressionNode{
                                ast::ExpressionNode{},
                                ast::BracketSpecifierNode{
                                    ast::ArrayItemNode{3}}}},
                        ast::BracketSpecifierNode{
                            ast::ArrayItemNode{4}}}},
                ast::BracketSpecifierNode{
                    ast::ArrayItemNode{5}}};
            String expression{"[3][4][5]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("subexpression after standalone index expression")
        {
            auto expectedResult = ast::SubexpressionNode{
                ast::ExpressionNode{
                    ast::IndexExpressionNode{
                        ast::ExpressionNode{},
                        ast::BracketSpecifierNode{
                            ast::ArrayItemNode{4}}}},
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}};
            String expression{"[4].\"id\""};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("index expresion after subexpression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{
                    ast::SubexpressionNode{
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id1"}},
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id2"}}}},
                ast::BracketSpecifierNode{
                    ast::ArrayItemNode{4}}};
            String expression{"\"id1\".\"id2\"[4]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("standalone flatten operator")
        {
            auto expectedResult = ast::IndexExpressionNode{
                    ast::ExpressionNode{},
                    ast::BracketSpecifierNode{
                        ast::FlattenOperatorNode{}}};
            String expression{"[]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("recursive standalone flatten operator")
        {
            auto expectedResult = ast::IndexExpressionNode{
                    ast::ExpressionNode{
                        ast::IndexExpressionNode{
                            ast::ExpressionNode{},
                            ast::BracketSpecifierNode{
                                ast::FlattenOperatorNode{}}}},
                    ast::BracketSpecifierNode{
                        ast::FlattenOperatorNode{}}};
            String expression{"[][]"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("standalone flatten operator with subexpression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                    ast::ExpressionNode{},
                    ast::BracketSpecifierNode{
                        ast::FlattenOperatorNode{}},
                    ast::ExpressionNode{
                        ast::SubexpressionNode{
                            ast::ExpressionNode{},
                            ast::ExpressionNode{
                                ast::IdentifierNode{"id"}}}}};
            String expression{"[].id"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("standalone flatten operator with recursive subexpression")
        {
            auto expectedResult = ast::IndexExpressionNode{
                    ast::ExpressionNode{},
                    ast::BracketSpecifierNode{
                        ast::FlattenOperatorNode{}},
                    ast::ExpressionNode{
                        ast::SubexpressionNode{
                            ast::ExpressionNode{
                                ast::SubexpressionNode{
                                    ast::ExpressionNode{
                                        ast::SubexpressionNode{
                                            ast::ExpressionNode{},
                                            ast::ExpressionNode{
                                                ast::IdentifierNode{"id1"}}}},
                                    ast::ExpressionNode{
                                        ast::IdentifierNode{"id2"}}}},
                            ast::ExpressionNode{
                                ast::IdentifierNode{"id3"}}}}};
            String expression{"[].id1.id2.id3"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }

        SECTION("recursive flatten operators with recursive subexpressions")
        {
            auto expectedResult = ast::IndexExpressionNode{
                ast::ExpressionNode{
                    ast::IndexExpressionNode{
                        ast::ExpressionNode{
                            ast::IdentifierNode{"id1"}},
                        ast::BracketSpecifierNode{
                            ast::FlattenOperatorNode{}},
                        ast::ExpressionNode{
                            ast::SubexpressionNode{
                                ast::ExpressionNode{
                                    ast::SubexpressionNode{
                                        ast::ExpressionNode{},
                                        ast::ExpressionNode{
                                            ast::IdentifierNode{"id2"}}}},
                                ast::ExpressionNode{
                                    ast::IdentifierNode{"id3"}}}}}},
                ast::BracketSpecifierNode{
                    ast::FlattenOperatorNode{}},
                ast::ExpressionNode{
                    ast::SubexpressionNode{
                        ast::ExpressionNode{
                            ast::SubexpressionNode{
                                ast::ExpressionNode{},
                                ast::ExpressionNode{
                                    ast::IdentifierNode{"id4"}}}},
                        ast::ExpressionNode{
                            ast::ExpressionNode{
                                ast::IdentifierNode{"id5"}}}}}};
            String expression{"id1[].id2.id3[].id4.id5"};

            REQUIRE(parseExpression(grammar, expression).expression
                    == expectedResult);
        }
    }
}

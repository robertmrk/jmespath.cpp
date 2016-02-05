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
#include "jmespath/parser/noderank.h"
#include "jmespath/ast/allnodes.h"

TEST_CASE("nodeRank")
{
    using namespace jmespath::parser;
    using namespace jmespath::ast;
    using namespace fakeit;

    SECTION("ranks basic nodes at 0")
    {
        REQUIRE(nodeRank(IdentifierNode{}) == 0);
        REQUIRE(nodeRank(RawStringNode{}) == 0);
        REQUIRE(nodeRank(LiteralNode{}) == 0);
    }

    SECTION("ranks empty expression node at -1")
    {
        REQUIRE(nodeRank(ExpressionNode{}) == -1);
    }

    SECTION("ranks non empty expression node with contained expression rank")
    {
        REQUIRE(nodeRank(ExpressionNode{IdentifierNode{}}) == 0);
    }

    SECTION("ranks subexpression node at 1")
    {
        REQUIRE(nodeRank(SubexpressionNode{}) == 1);
    }

    SECTION("ranks array item node at 1")
    {
        REQUIRE(nodeRank(ArrayItemNode{}) == 1);
    }

    SECTION("ranks flatten operator node at 2")
    {
        REQUIRE(nodeRank(FlattenOperatorNode{}) == 2);
    }

    SECTION("ranks bracket specifier node as its expression")
    {
        REQUIRE(nodeRank(BracketSpecifierNode{ArrayItemNode{}}) == 1);
        REQUIRE(nodeRank(BracketSpecifierNode{FlattenOperatorNode{}}) == 2);
    }

    SECTION("ranks index expression node as its bracket specifier")
    {
        IndexExpressionNode node1{
            BracketSpecifierNode{
                ArrayItemNode{}}};
        IndexExpressionNode node2{
            BracketSpecifierNode{
                FlattenOperatorNode{}}};

        REQUIRE(nodeRank(node1) == 1);
        REQUIRE(nodeRank(node2) == 2);
    }
}

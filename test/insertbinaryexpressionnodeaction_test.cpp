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
#include "jmespath/parser/insertbinaryexpressionnodeaction.h"
#include "jmespath/ast/allnodes.h"

TEST_CASE("InsertBinaryExpressionNodeAction")
{
    using namespace jmespath::parser;
    using namespace jmespath::ast;
    using namespace fakeit;

    InsertBinaryExpressionNodeAction action;

    SECTION("casts expressions containing valid binary nodes to binary nodes")
    {
        ExpressionNode node{SubexpressionNode{}};

        REQUIRE(action.toBinaryNode(&node) != nullptr);
    }

    SECTION("casts expressions containing non binary nodes to nullptr")
    {
        ExpressionNode node{IdentifierNode{}};

        REQUIRE(action.toBinaryNode(&node) == nullptr);
    }

    SECTION("returns nullptr if root is not a binary node")
    {
        ExpressionNode node{IdentifierNode{}};

        REQUIRE(action.leftmostBinaryNode(&node, 0) == nullptr);
    }

    SECTION("returns nullptr if suitable binary node can't be found")
    {
        ExpressionNode node{
            SubexpressionNode{
                ExpressionNode{},
                ExpressionNode{
                    IdentifierNode{}}}};

        REQUIRE(action.leftmostBinaryNode(&node,
                                          nodeRank(FlattenOperatorNode{}))
                == nullptr);
    }

    SECTION("finds leftmost binary node")
    {
        ExpressionNode node{
            IndexExpressionNode{
                ExpressionNode{
                    SubexpressionNode{
                        ExpressionNode{
                            IdentifierNode{}},
                        ExpressionNode{
                            IdentifierNode{}}}},
                BracketSpecifierNode{
                    FlattenOperatorNode{}},
                ExpressionNode{}}};
        auto index = boost::get<IndexExpressionNode>(&node.value);
        auto sub = boost::get<SubexpressionNode>(&index->leftExpression.value);

        auto result = action.leftmostBinaryNode(&node,
                                                nodeRank(SubexpressionNode{}));

        REQUIRE(result == sub);
    }

    SECTION("replaces target node with current node if target is not a binary"
            "node")
    {
        ExpressionNode targetNode;
        SubexpressionNode currentNode{
            ExpressionNode{},
            ExpressionNode{
                IdentifierNode{}}};

        action(targetNode, currentNode);

        REQUIRE(targetNode == ExpressionNode{currentNode});
    }

    SECTION("replaces target with current node and makes the target the right"
            "child of current node if current node has a higher rank")
    {
        ExpressionNode targetNode{
            SubexpressionNode{
                ExpressionNode{},
                ExpressionNode{
                    IdentifierNode{}}}};
        IndexExpressionNode currentNode{
            BracketSpecifierNode{
                FlattenOperatorNode{}}};
        ExpressionNode expectedResult{
            IndexExpressionNode{
                ExpressionNode{},
                BracketSpecifierNode{
                    FlattenOperatorNode{}},
                ExpressionNode{
                    SubexpressionNode{
                        ExpressionNode{},
                        ExpressionNode{
                            IdentifierNode{}}}}}};

        action(targetNode, currentNode);

        REQUIRE(targetNode == expectedResult);
    }

    SECTION("replaces target with current node and makes the target the right"
            "child of current node, if they have equal rank, current is a "
            "projection and target doesn't stops projections")
    {
        HashWildcardNode currentNode;
        ExpressionNode targetNode{
            IndexExpressionNode{
                BracketSpecifierNode{
                    ListWildcardNode{}}}};
        ExpressionNode expectedResult{
            HashWildcardNode{
                ExpressionNode{},
                ExpressionNode{
                    IndexExpressionNode{
                        BracketSpecifierNode{
                            ListWildcardNode{}}}}}};

        action(targetNode, currentNode);

        REQUIRE(targetNode == expectedResult);
    }

    SECTION("inserts projected current node and makes the left expression of"
            "the leftmost suitable node its right expression")
    {
        ExpressionNode targetNode{
            IndexExpressionNode{
                ExpressionNode{
                    SubexpressionNode{
                        ExpressionNode{},
                        ExpressionNode{
                            IdentifierNode{"id"}}}},
                BracketSpecifierNode{
                    FlattenOperatorNode{}},
                ExpressionNode{}}};
        IndexExpressionNode currentNode{
            BracketSpecifierNode{
                FlattenOperatorNode{}}};
        ExpressionNode expectedResult{
            IndexExpressionNode{
                ExpressionNode{
                    IndexExpressionNode{
                        ExpressionNode{},
                        BracketSpecifierNode{
                            FlattenOperatorNode{}},
                        ExpressionNode{
                            SubexpressionNode{
                                ExpressionNode{},
                                ExpressionNode{
                                    IdentifierNode{"id"}}}}}},
                BracketSpecifierNode{
                    FlattenOperatorNode{}},
                ExpressionNode{}}};

        action(targetNode, currentNode);

        REQUIRE(targetNode == expectedResult);
    }

    SECTION("inserts first expression as the left child of the leftmost"
            " binary node")
    {
        ExpressionNode rootNode{
            SubexpressionNode{
                ExpressionNode{},
                ExpressionNode{
                    IdentifierNode{}}}};
        IndexExpressionNode currentNode{
            BracketSpecifierNode{
                FlattenOperatorNode{}}};
        ExpressionNode firstExpression{
            LiteralNode{}};
        ExpressionNode expectedResult{
            IndexExpressionNode{
                ExpressionNode{
                    LiteralNode{}},
                BracketSpecifierNode{
                    FlattenOperatorNode{}},
                ExpressionNode{
                    SubexpressionNode{
                        ExpressionNode{},
                        ExpressionNode{
                            IdentifierNode{}}}}}};

        action(rootNode, currentNode, firstExpression);

        REQUIRE(rootNode == expectedResult);
    }
}

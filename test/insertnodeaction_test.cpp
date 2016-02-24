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
#include "jmespath/parser/insertnodeaction.h"
#include "jmespath/ast/allnodes.h"

class ChildExtractorStub
{
public:
    using result_type = jmespath::ast::ExpressionNode*;
    static bool called;

    result_type operator()(jmespath::ast::ExpressionNode*) const
    {
        called = true;
        return nullptr;
    }
};
bool ChildExtractorStub::called = false;

class NodeInserterStub
{
public:
    using result_type = void;
    static bool called;

    result_type operator()(jmespath::ast::ExpressionNode*,
                           jmespath::ast::AbstractNode*) const
    {
        called = true;
    }
};
bool NodeInserterStub::called = false;

class NodeInsertConditionStub
{
public:
    using result_type = bool;
    static bool called;
    static bool returnValue;

    result_type operator()(jmespath::ast::ExpressionNode*,
                           jmespath::ast::AbstractNode*) const
    {
        called = true;
        return returnValue;
    }
};
bool NodeInsertConditionStub::called = false;
bool NodeInsertConditionStub::returnValue = false;

TEST_CASE("InsertNodeAction")
{
    using namespace jmespath::parser;
    namespace ast = jmespath::ast;
    using namespace fakeit;

    InsertNodeAction<ChildExtractorStub,
            NodeInserterStub,
            NodeInsertConditionStub> insertAction;

    SECTION("ignores invalid target nodes")
    {
        ChildExtractorStub::called = false;
        NodeInserterStub::called = false;
        NodeInsertConditionStub::called = false;
        NodeInsertConditionStub::returnValue = false;
        ast::IdentifierNode node;

        insertAction(nullptr, node);

        REQUIRE_FALSE(ChildExtractorStub::called);
        REQUIRE_FALSE(NodeInserterStub::called);
        REQUIRE_FALSE(NodeInsertConditionStub::called);
    }

    SECTION("calls insert condition, and if it returns true calls the inserter")
    {
        ChildExtractorStub::called = false;
        NodeInserterStub::called = false;
        NodeInsertConditionStub::called = false;
        NodeInsertConditionStub::returnValue = true;
        ast::IdentifierNode node;
        ast::ExpressionNode targetNode;

        insertAction(targetNode, node);

        REQUIRE_FALSE(ChildExtractorStub::called);
        REQUIRE(NodeInserterStub::called);
        REQUIRE(NodeInsertConditionStub::called);
    }

    SECTION("calls insert condition, and if it returns false calls the "
            "extractor")
    {
        ChildExtractorStub::called = false;
        NodeInserterStub::called = false;
        NodeInsertConditionStub::called = false;
        NodeInsertConditionStub::returnValue = false;
        ast::IdentifierNode node;
        ast::ExpressionNode targetNode;

        insertAction(targetNode, node);

        REQUIRE(ChildExtractorStub::called);
        REQUIRE_FALSE(NodeInserterStub::called);
        REQUIRE(NodeInsertConditionStub::called);
    }
}

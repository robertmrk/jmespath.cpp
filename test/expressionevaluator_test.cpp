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
#include "jmespath/interpreter/expressionevaluator.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/detail/exceptions.h"
#include <boost/range/algorithm.hpp>

namespace jmespath { namespace ast {

std::ostream& operator<< (std::ostream& stream, ExpressionNode const&)
{
    return stream;
}
}} // namespace jmespath::ast

TEST_CASE("ExpressionEvaluator")
{
    using jmespath::interpreter::ExpressionEvaluator;    
    namespace ast = jmespath::ast;
    namespace rng = boost::range;
    using namespace jmespath::detail;
    using namespace fakeit;

    ExpressionEvaluator evaluator;

    SECTION("can be constructed with context value")
    {
        Json context{"ident", "value"};

        ExpressionEvaluator ev{context};

        REQUIRE(ev.currentContext() == context);
    }

    SECTION("accepts abstract node")
    {
        Mock<ast::AbstractNode> node;
        When(Method(node, accept).Using(&evaluator)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept)).Once();
    }

    SECTION("accepts expression node")
    {
        Mock<ast::ExpressionNode> node;
        When(Method(node, accept).Using(&evaluator)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept)).Once();
    }

    SECTION("evaluates identifier node")
    {
        ast::IdentifierNode node{"identifier"};
        int value = 15;
        Json expectedValue = value;
        REQUIRE(expectedValue.is_number());
        evaluator.setContext({{"identifier", value}});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedValue);
    }

    SECTION("evaluates non existing identifier to null")
    {
        ast::IdentifierNode node{"non-existing"};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates identifier on non object to null")
    {
        ast::IdentifierNode node{"identifier"};
        evaluator.setContext(15);

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates raw string")
    {
        String rawString{"[baz]"};
        ast::RawStringNode node{rawString};
        Json expectedValue = rawString;
        REQUIRE(expectedValue.is_string());

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedValue);
    }

    SECTION("evaluates literal")
    {
        String stringLiteralValue{"foo"};
        ast::LiteralNode stringNode{"\"" + stringLiteralValue + "\""};
        ast::LiteralNode arrayNode{"[1, 2, 3]"};
        Json expectedStringValue = stringLiteralValue;
        Json expectedArrayValue{1, 2, 3};
        REQUIRE(expectedStringValue.is_string());
        REQUIRE(expectedArrayValue.is_array());

        evaluator.visit(&stringNode);
        auto stringResult = evaluator.currentContext();
        evaluator.visit(&arrayNode);
        auto arrayResult = evaluator.currentContext();

        REQUIRE(stringResult == expectedStringValue);
        REQUIRE(arrayResult == expectedArrayValue);
    }

    SECTION("evaluates subexpression")
    {
        Mock<ast::SubexpressionNode> node;
        When(Method(node, accept)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept).Using(&evaluator)).Once();
    }    

    SECTION("evaluates bracket specifier")
    {
        Mock<ast::BracketSpecifierNode> node;
        When(Method(node, accept)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept).Using(&evaluator)).Once();
    }

    SECTION("evaluates index expression")
    {
        ast::IndexExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        evaluatorMock.get().setContext("[1, 2, 3]"_json);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::BracketSpecifierNode*)))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                  void(ast::BracketSpecifierNode*))
                    .Using(&node.bracketSpecifier))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates index expression on non array to null without evaluating"
            "bracket specifier")
    {
        ast::IndexExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        evaluatorMock.get().setContext("string value");
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        REQUIRE(evaluatorMock.get().currentContext() == Json{});
        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates projected index expression")
    {
        ast::IndexExpressionNode node{
            ast::ExpressionNode{},
            ast::BracketSpecifierNode{
                ast::FlattenOperatorNode{}},
            ast::ExpressionNode{}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        evaluatorMock.get().setContext("[1, 2, 3]"_json);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::BracketSpecifierNode*)))
                .AlwaysReturn();
        When(Method(evaluatorMock, evaluateProjection))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                  void(ast::BracketSpecifierNode*))
                    .Using(&node.bracketSpecifier)
               + Method(evaluatorMock, evaluateProjection)
                    .Using(&node.rightExpression))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates array item expression")
    {
        ast::ArrayItemNode node{2};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "two");
    }

    SECTION("evaluates array item expression with negative index")
    {
        ast::ArrayItemNode node{-4};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "one");
    }

    SECTION("evaluates array item expression on non arrays to null")
    {
        ast::ArrayItemNode node{2};
        evaluator.setContext(3);

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates array item expression with out of bounds index to null")
    {
        ast::ArrayItemNode node{15};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates projection")
    {
        Json context = {{{"id", 1}}, {{"id", 2}}, {{"id2", 3}}, {{"id", 4}}};
        REQUIRE(context.is_array());
        evaluator.setContext(context);
        ast::ExpressionNode expression{
            ast::IdentifierNode{"id"}};
        evaluator.setContext(context);
        Json expectedResult = {1, 2, 4};
        REQUIRE(expectedResult.is_array());

        evaluator.evaluateProjection(&expression);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates projection on non arrays to null")
    {
        Json context = "string";
        REQUIRE(context.is_string());
        ast::ExpressionNode expression{
            ast::IdentifierNode{"id"}};
        evaluator.setContext(context);

        evaluator.evaluateProjection(&expression);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates flatten operator")
    {
        Json context = "[1, 2, [3], [4, [5, 6, 7], 8], [9, 10] ]"_json;
        evaluator.setContext(context);
        Json expectedResult = "[1, 2, 3, 4, [5, 6, 7], 8, 9, 10]"_json;
        ast::FlattenOperatorNode flattenNode;

        evaluator.visit(&flattenNode);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates flatten operator on non array to null")
    {
        Json context = {{"id", "value"}};
        REQUIRE(context.is_object());
        evaluator.setContext(context);
        ast::FlattenOperatorNode flattenNode;

        evaluator.visit(&flattenNode);

        REQUIRE(evaluator.currentContext() == Json{});
    }


    SECTION("evaluates slice expression on non array to null")
    {
        ast::SliceExpressionNode sliceNode{2, 5};

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("throws InvalidValue exception when slice expression"
            "step equals to zero")
    {
        evaluator.setContext("[]"_json);
        ast::SliceExpressionNode sliceNode{2, 5, 0};

        REQUIRE_THROWS_AS(evaluator.visit(&sliceNode), InvalidValue);
    }

    SECTION("evaluates slice expression")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode{2, 5};
        Json expectedResult = "[2, 3, 4]"_json;

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates slice expression with step index")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode{2, 5, 2};
        Json expectedResult = "[2, 4]"_json;

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates slice expression with negative step index")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode{5, 2, -1};
        Json expectedResult = "[5, 4, 3]"_json;

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates empty slice expression")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode;

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == context);
    }

    SECTION("evaluates slice expression with end value over the end of array")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode{0, 20};

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == context);
    }

    SECTION("evaluates slice expression with start value below the first item"
            "of array")
    {
        Json context = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]"_json;
        evaluator.setContext(context);
        ast::SliceExpressionNode sliceNode{-50};

        evaluator.visit(&sliceNode);

        REQUIRE(evaluator.currentContext() == context);
    }

    SECTION("evaluates list wildcard expression on non array to null")
    {
        Json context = "string";
        evaluator.setContext(context);
        ast::ListWildcardNode node;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates list wildcard expression on arrays to the array itself")
    {
        Json context = "[1, 2, 3]"_json;
        evaluator.setContext(context);
        ast::ListWildcardNode node;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == context);
    }

    SECTION("evaluates hash wildcard expression on non object to null")
    {
        Json context = "[1, 2, 3]"_json;
        evaluator.setContext(context);
        ast::HashWildcardNode node;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates hash wildcard expression on objects to array of values")
    {
        Json context = "{\"a\": 1, \"b\":2, \"c\":3}"_json;
        Json values(Json::value_t::array);
        rng::copy(context, std::back_inserter(values));
        evaluator.setContext(context);
        ast::HashWildcardNode node;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == values);
    }

    SECTION("evaluates left expression of hash wildcard expresion and projects"
            "right expression")
    {
        ast::HashWildcardNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        When(Method(evaluatorMock, evaluateProjection))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + Method(evaluatorMock, evaluateProjection)
                    .Using(&node.rightExpression))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("visits child expressions of multiselect list expression on "
            "evaluation")
    {
        ast::ExpressionNode exp1{
            ast::IdentifierNode{"id1"}};
        ast::ExpressionNode exp2{
            ast::IdentifierNode{"id2"}};
        ast::ExpressionNode exp3{
            ast::IdentifierNode{"id3"}};
        ast::MultiselectListNode node{exp1, exp2, exp3};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        evaluatorMock.get().setContext("value");

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.expressions[0])
                + OverloadedMethod(evaluatorMock, visit,
                                   void(ast::ExpressionNode*))
                       .Using(&node.expressions[1])
                + OverloadedMethod(evaluatorMock, visit,
                                   void(ast::ExpressionNode*))
                       .Using(&node.expressions[2]))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("doesn't evaluates multiselect list expression on null context")
    {
        ast::ExpressionNode exp1{
            ast::IdentifierNode{"id1"}};
        ast::ExpressionNode exp2{
            ast::IdentifierNode{"id2"}};
        ast::ExpressionNode exp3{
            ast::IdentifierNode{"id3"}};
        ast::MultiselectListNode node{exp1, exp2, exp3};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates multiselect list by creating a list from the child"
            "expressions' results")
    {
        evaluator.setContext("{\"id1\": 1, \"id2\":2, \"id3\":3}"_json);
        ast::ExpressionNode exp1{
            ast::IdentifierNode{"id1"}};
        ast::ExpressionNode exp2{
            ast::IdentifierNode{"id2"}};
        ast::ExpressionNode exp3{
            ast::IdentifierNode{"id3"}};
        ast::MultiselectListNode node{exp1, exp2, exp3};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[1, 2, 3]"_json);
    }

    SECTION("visits child expressions of multiselect hash expression on "
            "evaluation")
    {
        ast::MultiselectHashNode node{
                {ast::IdentifierNode{"id1"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id2"}}},
                {ast::IdentifierNode{"id3"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id4"}}}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();
        evaluatorMock.get().setContext("value");

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
               .Using(&node.expressions[0].second)).Once();
        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
               .Using(&node.expressions[1].second)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("doesn't evaluates multiselect hash expression on null context")
    {
        ast::MultiselectHashNode node{
                {ast::IdentifierNode{"id1"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id2"}}},
                {ast::IdentifierNode{"id3"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id4"}}}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();
        evaluatorMock.get().setContext({});

        evaluatorMock.get().visit(&node);

        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates multiselect hash by creating an object from the child"
            "expressions' key and value")
    {
        evaluator.setContext("{\"id2\":\"value2\", \"id4\":\"value4\"}"_json);
        ast::MultiselectHashNode node{
                {ast::IdentifierNode{"id1"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id2"}}},
                {ast::IdentifierNode{"id3"},
                 ast::ExpressionNode{
                    ast::IdentifierNode{"id4"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "{\"id1\":\"value2\","
                                              "\"id3\":\"value4\"}"_json);
    }

    SECTION("evaluates child expression of not expression")
    {
        ast::NotExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
               .Using(&node.expression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates not expression on false to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":false}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on true to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":true}"_json);
        Json expectedResult = false;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on null to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":null}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on all numbers to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        Json expectedResult = false;

        evaluator.setContext("{\"id\":5}"_json);
        evaluator.visit(&node);
        auto result1 = evaluator.currentContext();
        evaluator.setContext("{\"id\":0}"_json);
        evaluator.visit(&node);
        auto result2 = evaluator.currentContext();
        evaluator.setContext("{\"id\":-5}"_json);
        evaluator.visit(&node);
        auto result3 = evaluator.currentContext();

        REQUIRE(result1 == expectedResult);
        REQUIRE(result2 == expectedResult);
        REQUIRE(result3 == expectedResult);
    }

    SECTION("evaluates not expression on empty string to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":\"\"}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on non empty string to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":\"string\"}"_json);
        Json expectedResult = false;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on empty array to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":[]}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on non empty array to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":[1, 2, 3]}"_json);
        Json expectedResult = false;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on empty object to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":{}}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on non empty object to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":{\"id2\":\"string\"}}"_json);
        Json expectedResult = false;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates comparator expression")
    {
        ast::ComparatorExpressionNode node{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::NotEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.rightExpression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates comparator expression with less operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::Less,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::Less,
            ast::ExpressionNode{
                ast::LiteralNode{"2"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();

        REQUIRE(result1);
        REQUIRE_FALSE(result2);
    }

    SECTION("evaluates comparator expression with less or equal operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::LessOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::LessOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"2"}}};
        ast::ComparatorExpressionNode node3{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::LessOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        Json result3 = evaluator.currentContext();

        REQUIRE(result1);
        REQUIRE_FALSE(result2);
        REQUIRE(result3);
    }

    SECTION("evaluates comparator expression with equal operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::Equal,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::Equal,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();

        REQUIRE(result1);
        REQUIRE_FALSE(result2);
    }

    SECTION("evaluates comparator expression with greater or equal operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::GreaterOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::GreaterOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"2"}}};
        ast::ComparatorExpressionNode node3{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::GreaterOrEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        Json result3 = evaluator.currentContext();

        REQUIRE_FALSE(result1);
        REQUIRE(result2);
        REQUIRE(result3);
    }

    SECTION("evaluates comparator expression with greater operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::Greater,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::Greater,
            ast::ExpressionNode{
                ast::LiteralNode{"2"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();

        REQUIRE_FALSE(result1);
        REQUIRE(result2);
    }

    SECTION("evaluates comparator expression with not equal operator")
    {
        ast::ComparatorExpressionNode node1{
            ast::ExpressionNode{
                ast::LiteralNode{"5"}},
            ast::ComparatorExpressionNode::Comparator::NotEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        ast::ComparatorExpressionNode node2{
            ast::ExpressionNode{
                ast::LiteralNode{"2"}},
            ast::ComparatorExpressionNode::Comparator::NotEqual,
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}};
        Json trueResult = true;

        evaluator.visit(&node1);
        Json result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        Json result2 = evaluator.currentContext();

        REQUIRE_FALSE(result1);
        REQUIRE(result2);
    }

    SECTION("throws exception on evaluating comparator expression with "
            "unknown operator")
    {
        ast::ComparatorExpressionNode node;

        REQUIRE_THROWS_AS(evaluator.visit(&node), InvalidAgrument);
    }

    SECTION("evaluates or expression")
    {
        ast::OrExpressionNode node{
            ast::ExpressionNode{},
            ast::ExpressionNode{}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.rightExpression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates or expression to left expression's result if it's true")
    {
        ast::OrExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id1"}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id2"}}};
        evaluator.setContext("{\"id1\": \"value1\", \"id2\": \"value2\"}"_json);
        Json expectedResult = "value1";

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates or expression to right expression's result if left "
            "expression's result is false")
    {
        ast::OrExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id1"}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id2"}}};
        evaluator.setContext("{\"id1\": \"\", \"id2\": \"value2\"}"_json);
        Json expectedResult = "value2";

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates and expression")
    {
        ast::AndExpressionNode node{
            ast::ExpressionNode{},
            ast::ExpressionNode{}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();
        evaluatorMock.get().setContext("true"_json);

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.rightExpression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates and expression to right expression's result if left "
            "expression's result is true")
    {
        ast::AndExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id1"}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id2"}}};
        evaluator.setContext("{\"id1\": \"value1\", \"id2\": \"value2\"}"_json);
        Json expectedResult = "value2";

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates and expression to left expression's result if left "
            "expression's result is false")
    {
        ast::AndExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id1"}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id2"}}};
        evaluator.setContext("{\"id1\": [], \"id2\": \"value2\"}"_json);
        Json expectedResult = "[]"_json;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates child expression of paren expression")
    {
        ast::ParenExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
               .Using(&node.expression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates pipe expression")
    {
        ast::PipeExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.rightExpression)).Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates pipe expression by passing the left expression's result "
            "to the right expression")
    {
        ast::PipeExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id1"}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id2"}}};
        evaluator.setContext("{\"id1\": {\"id2\": \"value\"}}"_json);
        Json expectedResult = "value";

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates current node expression")
    {
        ast::CurrentNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*))).AlwaysReturn();
        evaluatorMock.get().setContext("value");

        evaluatorMock.get().visit(&node);

        REQUIRE(evaluatorMock.get().currentContext() == "value");
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates current node inside subexpression")
    {
        ast::SubexpressionNode node{
            ast::ExpressionNode{
                ast::CurrentNode{}},
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\": \"value\"}"_json);
        Json expectedResult = "value";

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates child expression of filter expression")
    {
        ast::FilterExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        evaluatorMock.get().setContext("[1, 2, 3]"_json);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.expression)).Exactly(3);
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates filter expression on non array to null")
    {
        Json context = "{}"_json;
        evaluator.setContext(context);
        ast::FilterExpressionNode node;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates filter expression on arrays to an array filtered with "
            "the filter's subexpression")
    {
        Json context = "[{\"id\": 1}, {\"id\": 2}, {\"id\": 3}]"_json;
        evaluator.setContext(context);
        ast::FilterExpressionNode node{
            ast::ExpressionNode{
                ast::ComparatorExpressionNode{
                    ast::ExpressionNode{
                        ast::IdentifierNode{"id"}},
                    ast::ComparatorExpressionNode::Comparator::GreaterOrEqual,
                    ast::ExpressionNode{
                        ast::LiteralNode{"2"}}}}};
        Json expectedResult = "[{\"id\": 2}, {\"id\": 3}]"_json;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("function expression evaluation throws on non existing function "
            "call")
    {
        ast::FunctionExpressionNode node{"foo"};

        REQUIRE_THROWS_AS(evaluator.visit(&node), UnknownFunction);
    }

    SECTION("abs function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3"}},
            ast::ExpressionNode{
                ast::LiteralNode{"5"}}}};
        ast::FunctionExpressionNode node2{"abs"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("abs function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates abs function on integer")
    {
        ast::FunctionExpressionNode node1{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3"}}}};
        ast::FunctionExpressionNode node2{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"5"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "3"_json);
        REQUIRE(result2 == "5"_json);
    }

    SECTION("evaluates abs function on float")
    {
        ast::FunctionExpressionNode node1{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3.7"}}}};
        ast::FunctionExpressionNode node2{
            "abs",
            {ast::ExpressionNode{
                ast::LiteralNode{"5.8"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "3.7"_json);
        REQUIRE(result2 == "5.8"_json);
    }

    SECTION("avg function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "avg",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"avg"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("avg function throws on non array argument type")
    {
        ast::FunctionExpressionNode node{
            "avg",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("avg function throws on array argument non number item type")
    {
        ast::FunctionExpressionNode node{
            "avg",
            {ast::ExpressionNode{
                ast::LiteralNode{"[true, false]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates avg function")
    {
        ast::FunctionExpressionNode node{
            "avg",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 7.5, 4.3, -17.8]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "-1"_json);
    }

    SECTION("contains function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"contains"};
        ast::FunctionExpressionNode node1{
            "contains",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "contains",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("contains function throws on non array or non string argument type")
    {
        ast::FunctionExpressionNode node{
            "contains",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates contains function on arrays")
    {
        ast::FunctionExpressionNode node1{
            "contains",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 7.5, 4.3, -17.8]"}},
            ast::ExpressionNode{
                ast::LiteralNode{"2"}}}};
        ast::FunctionExpressionNode node2{
            "contains",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 7.5, 4.3, -17.8]"}},
            ast::ExpressionNode{
                ast::LiteralNode{"3"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "true"_json);
        REQUIRE(result2 == "false"_json);
    }

    SECTION("evaluates contains function on strings")
    {
        ast::FunctionExpressionNode node1{
            "contains",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox...\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"fox\""}}}};
        ast::FunctionExpressionNode node2{
            "contains",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox...\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"dog\""}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "true"_json);
        REQUIRE(result2 == "false"_json);
    }

    SECTION("ceil function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "ceil",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"ceil"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("ceil function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "ceil",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates ceil function on integer")
    {
        ast::FunctionExpressionNode node1{
            "ceil",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3"}}}};
        ast::FunctionExpressionNode node2{
            "ceil",
            {ast::ExpressionNode{
                ast::LiteralNode{"5"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "-3"_json);
        REQUIRE(result2 == "5"_json);
    }

    SECTION("evaluates ceil function on float")
    {
        ast::FunctionExpressionNode node1{
            "ceil",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3.7"}}}};
        ast::FunctionExpressionNode node2{
            "ceil",
            {ast::ExpressionNode{
                ast::LiteralNode{"5.8"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "-3"_json);
        REQUIRE(result2 == "6"_json);
    }

    SECTION("ends_with function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"ends_with"};
        ast::FunctionExpressionNode node1{
            "ends_with",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "ends_with",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("ends_with function throws on non string argument types")
    {
        ast::FunctionExpressionNode node1{
            "ends_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node2{
            "ends_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "ends_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates ends_with function")
    {
        ast::FunctionExpressionNode node1{
            "ends_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"fox\""}}}};
        ast::FunctionExpressionNode node2{
            "ends_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"dog\""}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "true"_json);
        REQUIRE(result2 == "false"_json);
    }

    SECTION("floor function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "floor",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"floor"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("floor function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "floor",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates floor function on integer")
    {
        ast::FunctionExpressionNode node1{
            "floor",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3"}}}};
        ast::FunctionExpressionNode node2{
            "floor",
            {ast::ExpressionNode{
                ast::LiteralNode{"5"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "-3"_json);
        REQUIRE(result2 == "5"_json);
    }

    SECTION("evaluates floor function on float")
    {
        ast::FunctionExpressionNode node1{
            "floor",
            {ast::ExpressionNode{
                ast::LiteralNode{"-3.7"}}}};
        ast::FunctionExpressionNode node2{
            "floor",
            {ast::ExpressionNode{
                ast::LiteralNode{"5.8"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "-4"_json);
        REQUIRE(result2 == "5"_json);
    }

    SECTION("join function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"join"};
        ast::FunctionExpressionNode node1{
            "join",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "join",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("join function throws on non string and string array types")
    {
        ast::FunctionExpressionNode node1{
            "join",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node2{
            "join",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "join",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"[\"string\"]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates join function")
    {
        ast::FunctionExpressionNode node{
            "join",
            {ast::ExpressionNode{
                ast::LiteralNode{"\";\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"[\"string1\", \"string2\", \"string3\"]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext()
                == "\"string1;string2;string3\""_json);
    }

    SECTION("keys function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "keys",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"keys"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("keys function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "keys",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates keys function")
    {
        ast::FunctionExpressionNode node{
            "keys",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id2\": 2}"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[\"id1\", \"id2\"]"_json);
    }

    SECTION("length function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "length",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"length"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("length function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "length",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates length function")
    {
        ast::FunctionExpressionNode node1{
            "length",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id2\": 2}"}}}};
        ast::FunctionExpressionNode node2{
            "length",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};
        ast::FunctionExpressionNode node3{
            "length",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();

        REQUIRE(result1 == "2"_json);
        REQUIRE(result2 == "3"_json);
        REQUIRE(result3 == "6"_json);
    }

    SECTION("map function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"map"};
        ast::FunctionExpressionNode node1{
            "map",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "map",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("map function throws on invalid argument types")
    {
        ast::FunctionExpressionNode node1{
            "map",
            {ast::ExpressionNode{},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node2{
            "map",
            {ast::ExpressionArgumentNode{
                ast::ExpressionNode{}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "map",
            {ast::ExpressionNode{},
            ast::ExpressionNode{
                ast::LiteralNode{"[]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates map function")
    {
        ast::FunctionExpressionNode node{
            "map",
            {ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}},
            ast::ExpressionNode{
                ast::LiteralNode{"[{\"id\": 1}, {\"id\": 2}, {\"id2\": 3}]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[1, 2, null]"_json);
    }

    SECTION("max function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "max",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"max"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("max function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "max",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("max function throws on array argument invalid item type")
    {
        ast::FunctionExpressionNode node{
            "max",
            {ast::ExpressionNode{
                ast::LiteralNode{"[true, false]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("max function throws on array argument heterogeneous item types")
    {
        ast::FunctionExpressionNode node{
            "max",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, \"string\"]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates max function on number array")
    {
        ast::FunctionExpressionNode node{
            "max",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 3, 1]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "3"_json);
    }

    SECTION("evaluates max function on string array")
    {
        ast::FunctionExpressionNode node{
            "max",
            {ast::ExpressionNode{
                ast::LiteralNode{"[\"bar\", \"foo\", \"baz\"]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "\"foo\""_json);
    }

    SECTION("max_by function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"max_by"};
        ast::FunctionExpressionNode node1{
            "max_by",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "max_by",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("max_by function throws on invalid argument types")
    {
        ast::FunctionExpressionNode node1{
            "max_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{
            "max_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "max_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("max_by function throws on invalid expression result type")
    {
        ast::FunctionExpressionNode node{
            "max_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates max_by function")
    {
        ast::FunctionExpressionNode node{
            "max_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[{\"id\": 3}, {\"id\": 5}, {\"id\": 1}]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "{\"id\": 5}"_json);
    }

    SECTION("merge function throws on non object argument types")
    {
        ast::FunctionExpressionNode node{
            "merge",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id\": 2}"}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates merge function on objects")
    {
        ast::FunctionExpressionNode node{
            "merge",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 0, \"id2\": 2}"}},
            ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id3\": 3}"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext()
                == "{\"id1\": 1, \"id2\":2, \"id3\": 3}"_json);
    }

    SECTION("min function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "min",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"min"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("min function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "min",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("min function throws on array argument invalid item type")
    {
        ast::FunctionExpressionNode node{
            "min",
            {ast::ExpressionNode{
                ast::LiteralNode{"[true, false]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("min function throws on array argument heterogeneous item types")
    {
        ast::FunctionExpressionNode node{
            "min",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, \"string\"]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates min function on number array")
    {
        ast::FunctionExpressionNode node{
            "min",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 3, 1]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "1"_json);
    }

    SECTION("evaluates min function on string array")
    {
        ast::FunctionExpressionNode node{
            "min",
            {ast::ExpressionNode{
                ast::LiteralNode{"[\"bar\", \"foo\", \"baz\"]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "\"bar\""_json);
    }

    SECTION("min_by function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"min_by"};
        ast::FunctionExpressionNode node1{
            "min_by",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "min_by",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("min_by function throws on invalid argument types")
    {
        ast::FunctionExpressionNode node1{
            "min_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{
            "min_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "min_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("min_by function throws on invalid expression result type")
    {
        ast::FunctionExpressionNode node{
            "min_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates min_by function")
    {
        ast::FunctionExpressionNode node{
            "min_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[{\"id\": 3}, {\"id\": 5}, {\"id\": 1}]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "{\"id\": 1}"_json);
    }

    SECTION("not_null function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"not_null"};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
    }

    SECTION("not_null function throws on non JSON argument type")
    {
        ast::FunctionExpressionNode node{
            "not_null",
            {ast::ExpressionArgumentNode{
                ast::ExpressionNode{}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates not_null function")
    {
        ast::FunctionExpressionNode node{
            "not_null",
            {ast::ExpressionNode{
                ast::LiteralNode{"null"}},
            ast::ExpressionNode{
                ast::LiteralNode{"[]"}},
            ast::ExpressionNode{
                ast::LiteralNode{"null"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[]"_json);
    }

    SECTION("reverse function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"reverse"};
        ast::FunctionExpressionNode node2{
            "reverse",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("reverse function throws on non array or non string argument type")
    {
        ast::FunctionExpressionNode node{
            "reverse",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates reverse function on array")
    {
        ast::FunctionExpressionNode node{
            "reverse",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[3, 2, 1]"_json);
    }

    SECTION("evaluates reverse function on strings")
    {
        ast::FunctionExpressionNode node{
            "reverse",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"abc\""}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "\"cba\""_json);
    }

    SECTION("sort function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"sort"};
        ast::FunctionExpressionNode node2{
            "sort",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("sort function throws on non array type")
    {
        ast::FunctionExpressionNode node{
            "sort",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("sort function throws on non number or string item type")
    {
        ast::FunctionExpressionNode node{
            "sort",
            {ast::ExpressionNode{
                ast::LiteralNode{"[true, false]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates sort function on array of numbers")
    {
        ast::FunctionExpressionNode node{
            "sort",
            {ast::ExpressionNode{
                ast::LiteralNode{"[3, 1, 2]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[1, 2, 3]"_json);
    }

    SECTION("evaluates sort function on array of strings")
    {
        ast::FunctionExpressionNode node{
            "sort",
            {ast::ExpressionNode{
                ast::LiteralNode{"[\"b\", \"c\", \"a\"]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[\"a\", \"b\", \"c\"]"_json);
    }

    SECTION("sort_by function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"sort_by"};
        ast::FunctionExpressionNode node1{
            "sort_by",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "sort_by",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("sort_by function throws on invalid argument types")
    {
        ast::FunctionExpressionNode node1{
            "sort_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{
            "sort_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "sort_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("sort_by function throws on invalid expression result type")
    {
        ast::FunctionExpressionNode node{
            "sort_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates sort_by function")
    {
        ast::FunctionExpressionNode node{
            "sort_by",
            {ast::ExpressionNode{
                ast::LiteralNode{"[{\"id\": 3}, {\"id\": 5}, {\"id\": 1}]"}},
            ast::ExpressionArgumentNode{
                ast::ExpressionNode{
                    ast::IdentifierNode{"id"}}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext()
                == "[{\"id\": 1}, {\"id\": 3}, {\"id\": 5}]"_json);
    }

    SECTION("starts_with function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node0{"starts_with"};
        ast::FunctionExpressionNode node1{
            "starts_with",
            {ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node3{
            "starts_with",
            {ast::ExpressionNode{},
            ast::ExpressionNode{},
            ast::ExpressionNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node0),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentArity);
    }

    SECTION("starts_with function throws on non string argument types")
    {
        ast::FunctionExpressionNode node1{
            "starts_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node2{
            "starts_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "starts_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentType);
        REQUIRE_THROWS_AS(evaluator.visit(&node3),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates starts_with function")
    {
        ast::FunctionExpressionNode node1{
            "starts_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"The\""}}}};
        ast::FunctionExpressionNode node2{
            "starts_with",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"The quick brown fox\""}},
            ast::ExpressionNode{
                ast::LiteralNode{"\"fox\""}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();

        REQUIRE(result1 == "true"_json);
        REQUIRE(result2 == "false"_json);
    }

    SECTION("sum function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "sum",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"sum"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("sum function throws on non array argument type")
    {
        ast::FunctionExpressionNode node{
            "sum",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("sum function throws on array argument non number item type")
    {
        ast::FunctionExpressionNode node{
            "sum",
            {ast::ExpressionNode{
                ast::LiteralNode{"[true, false]"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates sum function")
    {
        ast::FunctionExpressionNode node{
            "sum",
            {ast::ExpressionNode{
                ast::LiteralNode{"[2, 7.5, 4.3, -17.8]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "-4"_json);
    }

    SECTION("to_array function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "to_array",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"to_array"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("to_array function throws on non JSON argument type")
    {
        ast::FunctionExpressionNode node{
            "to_array",
            {ast::ExpressionArgumentNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates to_array function on array to the array itself")
    {
        ast::FunctionExpressionNode node{
            "to_array",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[1, 2, 3]"_json);
    }

    SECTION("evaluates to_array function on non array to a single element "
            "array")
    {
        ast::FunctionExpressionNode node1{
            "to_array",
            {ast::ExpressionNode{
                ast::LiteralNode{"1"}}}};
        ast::FunctionExpressionNode node2{
            "to_array",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "to_array",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};
        ast::FunctionExpressionNode node4{
            "to_array",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id\": 1}"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();
        evaluator.visit(&node4);
        auto result4 = evaluator.currentContext();

        REQUIRE(result1 == "[1]"_json);
        REQUIRE(result2 == "[true]"_json);
        REQUIRE(result3 == "[\"string\"]"_json);
        REQUIRE(result4 == "[{\"id\": 1}]"_json);
    }

    SECTION("to_string function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "to_string",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"to_string"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("to_string function throws on non JSON argument type")
    {
        ast::FunctionExpressionNode node{
            "to_string",
            {ast::ExpressionArgumentNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates to_string function on string to the string itself")
    {
        ast::FunctionExpressionNode node{
            "to_string",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "\"string\""_json);
    }

    SECTION("evaluates to_string function on non string to the string "
            "representation of argument")
    {
        ast::FunctionExpressionNode node1{
            "to_string",
            {ast::ExpressionNode{
                ast::LiteralNode{"1"}}}};
        ast::FunctionExpressionNode node2{
            "to_string",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node3{
            "to_string",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};
        ast::FunctionExpressionNode node4{
            "to_string",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id2\": 2}"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();
        evaluator.visit(&node4);
        auto result4 = evaluator.currentContext();

        REQUIRE(result1 == "\"1\""_json);
        REQUIRE(result2 == "\"true\""_json);
        REQUIRE(result3 == "\"[1,2,3]\""_json);
        REQUIRE(result4 == "\"{\\\"id1\\\":1,\\\"id2\\\":2}\""_json);
    }

    SECTION("to_number function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "to_number",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"to_number"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("to_number function throws on non JSON argument type")
    {
        ast::FunctionExpressionNode node{
            "to_number",
            {ast::ExpressionArgumentNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates to_number function on number and string to number")
    {
        ast::FunctionExpressionNode node1{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"1.2"}}}};
        ast::FunctionExpressionNode node2{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"-2.7\""}}}};
        ast::FunctionExpressionNode node3{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"not a number\""}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();

        REQUIRE(result1 == "1.2"_json);
        REQUIRE(result2 == "-2.7"_json);
        REQUIRE(result3 == "null"_json);
    }

    SECTION("evaluates to_number function on non numbers and strings to null")
    {
        ast::FunctionExpressionNode node1{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node2{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};
        ast::FunctionExpressionNode node3{
            "to_number",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id2\": 2}"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();

        REQUIRE(result1 == "null"_json);
        REQUIRE(result2 == "null"_json);
        REQUIRE(result3 == "null"_json);
    }

    SECTION("type function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "type",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"type"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("type function throws on non JSON argument type")
    {
        ast::FunctionExpressionNode node{
            "type",
            {ast::ExpressionArgumentNode{}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates type function to the name of the JSON value type")
    {
        ast::FunctionExpressionNode node1{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"1.2"}}}};
        ast::FunctionExpressionNode node2{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"\"string\""}}}};
        ast::FunctionExpressionNode node3{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};
        ast::FunctionExpressionNode node4{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"[1, 2, 3]"}}}};
        ast::FunctionExpressionNode node5{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id\": 1}"}}}};
        ast::FunctionExpressionNode node6{
            "type",
            {ast::ExpressionNode{
                ast::LiteralNode{"null"}}}};

        evaluator.visit(&node1);
        auto result1 = evaluator.currentContext();
        evaluator.visit(&node2);
        auto result2 = evaluator.currentContext();
        evaluator.visit(&node3);
        auto result3 = evaluator.currentContext();
        evaluator.visit(&node4);
        auto result4 = evaluator.currentContext();
        evaluator.visit(&node5);
        auto result5 = evaluator.currentContext();
        evaluator.visit(&node6);
        auto result6 = evaluator.currentContext();

        REQUIRE(result1 == "\"number\""_json);
        REQUIRE(result2 == "\"string\""_json);
        REQUIRE(result3 == "\"boolean\""_json);
        REQUIRE(result4 == "\"array\""_json);
        REQUIRE(result5 == "\"object\""_json);
        REQUIRE(result6 == "\"null\""_json);
    }

    SECTION("values function throws on invalid number of arguments")
    {
        ast::FunctionExpressionNode node1{
            "values",
            {ast::ExpressionNode{},
            ast::ExpressionNode{}}};
        ast::FunctionExpressionNode node2{"values"};

        REQUIRE_THROWS_AS(evaluator.visit(&node1),
                          InvalidFunctionArgumentArity);
        REQUIRE_THROWS_AS(evaluator.visit(&node2),
                          InvalidFunctionArgumentArity);
    }

    SECTION("values function throws on inavlid argument type")
    {
        ast::FunctionExpressionNode node{
            "values",
            {ast::ExpressionNode{
                ast::LiteralNode{"true"}}}};

        REQUIRE_THROWS_AS(evaluator.visit(&node),
                          InvalidFunctionArgumentType);
    }

    SECTION("evaluates values function")
    {
        ast::FunctionExpressionNode node{
            "values",
            {ast::ExpressionNode{
                ast::LiteralNode{"{\"id1\": 1, \"id2\": 2}"}}}};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "[1, 2]"_json);
    }
}

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
#include "jmespath/ast/allnodes.h"
#include "jmespath/detail/exceptions.h"
#include <boost/range/algorithm.hpp>
#define private public
#include "jmespath/interpreter/expressionevaluator.h"

TEST_CASE("ExpressionEvaluator")
{
    using jmespath::interpreter::ExpressionEvaluator;
    using jmespath::detail::Json;
    using jmespath::detail::String;
    using jmespath::detail::InvalidValue;
    using jmespath::detail::InvalidAgrument;
    namespace ast = jmespath::ast;
    namespace rng = boost::range;
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

    SECTION("doesn't adjusts positive slice endpoint between bounds")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, 2, 1) == 2);
    }

    SECTION("adjusts negative slice endopint to index relative to the end")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, -2, 1) == 3);
    }

    SECTION("adjusts out of bounds negative index to 0 on positive step index")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, -10, 1) == 0);
    }

    SECTION("adjusts out of bounds negative index to -1 on negative step index")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, -10, -1) == -1);
    }

    SECTION("adjusts out of bounds positive index to end index on positive step"
            " index")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, 10, 1) == 5);
    }

    SECTION("adjusts out of bounds positive index to last valid index on "
            "negative step index")
    {
        REQUIRE(evaluator.adjustSliceEndpoint(5, 10, -1) == 4);
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

    SECTION("converts JSON true value to true")
    {
        REQUIRE(evaluator.toBoolean("true"_json));
    }

    SECTION("converts JSON false value to false")
    {
        REQUIRE_FALSE(evaluator.toBoolean("false"_json));
    }

    SECTION("converts all JSON numbers to true")
    {
        REQUIRE(evaluator.toBoolean("-1"_json));
        REQUIRE(evaluator.toBoolean("0"_json));
        REQUIRE(evaluator.toBoolean("1"_json));
    }

    SECTION("converts empty JSON string to false")
    {
        REQUIRE_FALSE(evaluator.toBoolean("\"\""_json));
    }

    SECTION("converts non empty JSON string to true")
    {
        REQUIRE(evaluator.toBoolean("\"string\""_json));
    }

    SECTION("converts empty JSON array to false")
    {
        REQUIRE_FALSE(evaluator.toBoolean("[]"_json));
    }

    SECTION("converts empty JSON object to false")
    {
        REQUIRE_FALSE(evaluator.toBoolean("{}"_json));
    }

    SECTION("converts non empty JSON array to true")
    {
        REQUIRE(evaluator.toBoolean("[1, 2, 3]"_json));
    }

    SECTION("converts non empty JSON object to true")
    {
        REQUIRE(evaluator.toBoolean("{\"id\": 1}"_json));
    }

    SECTION("evaluates not expression on false value to true")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":false}"_json);
        Json expectedResult = true;

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates not expression on true value to false")
    {
        ast::NotExpressionNode node{
            ast::ExpressionNode{
                ast::IdentifierNode{"id"}}};
        evaluator.setContext("{\"id\":true}"_json);
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
}

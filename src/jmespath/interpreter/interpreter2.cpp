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
#include "jmespath/interpreter/interpreter2.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/exceptions.h"
#include "jmespath/interpreter/contextvaluevisitoradaptor.h"
#include <numeric>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/hana.hpp>

namespace jmespath { namespace interpreter {

namespace rng = boost::range;
namespace alg = boost::algorithm;

template <typename NodeT>
using ContextVisitor = ContextValueVisitorAdaptor<NodeT>;

Interpreter2::Interpreter2(const Json &value)
    : AbstractVisitor{}, m_context2{assignContextValue(value)}
{
    // initialize JMESPath function name to function implementation mapping
    using std::placeholders::_1;
    using std::bind;
    auto exactlyOne = bind(std::equal_to<size_t>{}, _1, 1);
    auto exactlyTwo = bind(std::equal_to<size_t>{}, _1, 2);
    auto zeroOrMore = bind(std::greater_equal<size_t>{}, _1, 0);
    auto oneOrMore = bind(std::greater_equal<size_t>{}, _1, 1);
    m_functionMap = {
        {"abs", {exactlyOne, bind(&Interpreter2::abs, this, _1)}},
        {"avg", {exactlyOne, bind(&Interpreter2::avg, this, _1)}},
        {"contains", {exactlyTwo, bind(&Interpreter2::contains, this, _1)}},
        {"ceil", {exactlyOne, bind(&Interpreter2::ceil, this, _1)}},
        {"ends_with", {exactlyTwo, bind(&Interpreter2::endsWith, this, _1)}},
        {"floor", {exactlyOne, bind(&Interpreter2::floor, this, _1)}},
        {"join", {exactlyTwo, bind(&Interpreter2::join, this, _1)}},
        {"keys", {exactlyOne, bind(&Interpreter2::keys, this, _1)}},
        {"length", {exactlyOne, bind(&Interpreter2::length, this, _1)}},
        {"map", {exactlyTwo, bind(&Interpreter2::map, this, _1)}},
        {"max", {exactlyOne,
                 bind(&Interpreter2::max, this, _1, std::less<Json>{})}},
        {"max_by", {exactlyTwo,
                    bind(&Interpreter2::maxBy, this, _1, std::less<Json>{})}},
        {"merge", {zeroOrMore, bind(&Interpreter2::merge, this, _1)}},
        {"min", {exactlyOne,
                 bind(&Interpreter2::max, this, _1, std::greater<Json>{})}},
        {"min_by", {exactlyTwo,
                    bind(&Interpreter2::maxBy, this, _1, std::greater<Json>{})}},
        {"not_null", {oneOrMore, bind(&Interpreter2::notNull, this, _1)}},
        {"reverse", {exactlyOne, bind(&Interpreter2::reverse, this, _1)}},
        {"sort", {exactlyOne, bind(&Interpreter2::sort, this, _1)}},
        {"sort_by", {exactlyTwo, bind(&Interpreter2::sortBy, this, _1)}},
        {"starts_with", {exactlyTwo, bind(&Interpreter2::startsWith, this, _1)}},
        {"sum", {exactlyOne, bind(&Interpreter2::sum, this, _1)}},
        {"to_array", {exactlyOne, bind(&Interpreter2::toArray, this, _1)}},
        {"to_string", {exactlyOne, bind(&Interpreter2::toString, this, _1)}},
        {"to_number", {exactlyOne, bind(&Interpreter2::toNumber, this, _1)}},
        {"type", {exactlyOne, bind(&Interpreter2::type, this, _1)}},
        {"values", {exactlyOne, bind(&Interpreter2::values, this, _1)}}
    };
}

Interpreter2::Interpreter2(Json &&value)
    : AbstractVisitor ()
{
    m_context2 = value;
}

void Interpreter2::setContext(const Json &value)
{
    m_context = value;
}

Json Interpreter2::currentContext() const
{
    return m_context;
}

void Interpreter2::evaluateProjection(const ast::ExpressionNode *expression)
{
    // move the current context into a temporary variable in case it holds
    // a value, since the context member variable will get overwritten during
    // the evaluation of the projection
    ContextValue contextValue {std::move(m_context2)};

    // project the expression with either an lvalue const ref or rvalue ref
    // context
    auto visitor = boost::hana::overload(
        [this, &expression](const JsonRef& value) {
            evaluateProjection(expression, value.get());
        },
        [this, &expression](Json& value) {
            evaluateProjection(expression, std::move(value));
        }
    );
    boost::apply_visitor(visitor, contextValue);
}

template <typename JsonT>
void Interpreter2::evaluateProjection(const ast::ExpressionNode* expression,
                                      JsonT&& context)
{
    // evaluate the projection if the context holds an array
    if (context.is_array())
    {
        // create the array of results
        Json result(Json::value_t::array);
        // iterate over the array
        for (auto& item: context)
        {
            // move the item into the context or create an lvalue reference
            // depending on the type of the context variable
            m_context2 = assignContextValue(std::move(item));
            // evaluate the expression
            visit(expression);
            // if the result of the expression is not null
            if (!getJsonValue(m_context2).is_null())
            {
                // append a copy of the result if it's an lvalue reference
                // otherwise move it
                auto visitor = boost::hana::overload(
                    [&result](const JsonRef& value) {
                        result.push_back(value.get());
                    },
                    [&result](Json& value) {
                        result.push_back(std::move(value));
                    }
                );
                boost::apply_visitor(visitor, m_context2);
            }
        }

        // set the results of the projection
        m_context2 = std::move(result);
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::AbstractNode *node)
{
    node->accept(this);
}

void Interpreter2::visit(const ast::ExpressionNode *node)
{
    node->accept(this);
}

void Interpreter2::visit(const ast::IdentifierNode *node)
{
    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::IdentifierNode>(this, node),
                         m_context2);
}

template <typename JsonT>
void Interpreter2::visit(const ast::IdentifierNode *node, JsonT &&context)
{
    // evaluete the identifier if the context holds an object
    if (context.is_object())
    {
        try
        {
            // assign either a const reference of the result or move the result
            // into the context depending on the type of the context parameter
            m_context2 = assignContextValue(std::move(
                                                context.at(node->identifier)));
            return;
        }
        catch (const nlohmann::json::out_of_range&) {}
    }
    // otherwise evaluate to null
    m_context2 = {};
}

void Interpreter2::visit(const ast::RawStringNode *node)
{
    m_context2 = node->rawString;
}

void Interpreter2::visit(const ast::LiteralNode *node)
{
    m_context2 = Json::parse(node->literal);
}

void Interpreter2::visit(const ast::SubexpressionNode *node)
{
    node->accept(this);
}

void Interpreter2::visit(const ast::IndexExpressionNode *node)
{
    // evaluate the left side expression
    visit(&node->leftExpression);
    // evaluate the index expression if the context holds an array
    if (getJsonValue(m_context2).is_array())
    {
        // evaluate the bracket specifier
        visit(&node->bracketSpecifier);
        // if the index expression also defines a projection then evaluate it
        if (node->isProjection())
        {
            evaluateProjection(&node->rightExpression);
        }
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::ArrayItemNode *node)
{
    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::ArrayItemNode>(this, node),
                         m_context2);
}

template <typename JsonT>
void Interpreter2::visit(const ast::ArrayItemNode *node, JsonT &&context)
{
    // evaluate the array item expression if the context holds an array
    if (context.is_array())
    {
        // normalize the index value
        auto arrayIndex = node->index;
        if (arrayIndex < 0)
        {
            arrayIndex += context.size();
        }

        // evaluate the expression if the index is not out of range
        if ((arrayIndex >= 0) && (arrayIndex < context.size()))
        {
            // assign either a const reference of the result or move the result
            // into the context depending on the type of the context parameter
            auto index = static_cast<size_t>(arrayIndex);
            m_context2 = assignContextValue(std::move(context[index]));
            return;
        }
    }
    // otherwise evaluate to null
    m_context2 = {};
}

void Interpreter2::visit(const ast::FlattenOperatorNode *node)
{
    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::FlattenOperatorNode>(this, node),
                         m_context2);
}

template <typename JsonT>
void Interpreter2::visit(const ast::FlattenOperatorNode*, JsonT&& context)
{
    // evaluate the flatten operation if the context holds an array
    if (context.is_array())
    {
        // create the array of results
        Json result(Json::value_t::array);
        // iterate over the array
        for (auto& item: context)
        {
            // if the item is an array append or move every one of its items
            // to the end of the results variable
            if (item.is_array())
            {
                std::move(std::begin(item),
                          std::end(item),
                          std::back_inserter(result));
            }
            // otherwise append or move the item
            else
            {
                result.push_back(std::move(item));
            }
        }
        // set the results of the flatten operation
        m_context2 = std::move(result);
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::BracketSpecifierNode *node)
{
    node->accept(this);
}

void Interpreter2::visit(const ast::SliceExpressionNode *node)
{
    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::SliceExpressionNode>(this, node),
                         m_context2);
}

template <typename JsonT>
void Interpreter2::visit(const ast::SliceExpressionNode* node, JsonT&& context)
{
    // evaluate the slice operation if the context holds an array
    if (context.is_array())
    {
        Index startIndex = 0;
        Index stopIndex = 0;
        Index step = 1;
        size_t length = context.size();

        // verify the validity of slice indeces and normalize their values
        if (node->step)
        {
            if (*node->step == 0)
            {
                BOOST_THROW_EXCEPTION(InvalidValue{});
            }
            step = *node->step;
        }
        if (!node->start)
        {
            startIndex = step < 0 ? length - 1: 0;
        }
        else
        {
            startIndex = adjustSliceEndpoint(length, *node->start, step);
        }
        if (!node->stop)
        {
            stopIndex = step < 0 ? -1 : Index{length};
        }
        else
        {
            stopIndex = adjustSliceEndpoint(length, *node->stop, step);
        }

        // create the array of results
        Json result(Json::value_t::array);
        // iterate over the array
        for (auto i = startIndex;
             step > 0 ? (i < stopIndex) : (i > stopIndex);
             i += step)
        {
            // append a copy of the item at arrayIndex or move it into the
            // result array depending on the type of the context variable
            size_t arrayIndex = static_cast<size_t>(i);
            result.push_back(std::move(context[arrayIndex]));
        }

        // set the results of the projection
        m_context2 = std::move(result);
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::ListWildcardNode*)
{
    // evaluate a list wildcard operation to null if the context isn't an array
    if (!getJsonValue(m_context2).is_array())
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::HashWildcardNode *node)
{
    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::HashWildcardNode>(this, node),
                         m_context2);
}

template <typename JsonT>
void Interpreter2::visit(const ast::HashWildcardNode* node, JsonT&& context)
{
    // evaluate the left side expression
    visit(&node->leftExpression);
    // evaluate the hash wildcard operation if the context holds an array
    if (context.is_object())
    {
        // create the array of results
        Json result(Json::value_t::array);
        // move or copy every value in the object into the list of results
        std::move(std::begin(context),
                  std::end(context),
                  std::back_inserter(result));

        // set the results of the projection
        m_context2 = std::move(result);
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
    // evaluate the projected sub expression
    evaluateProjection(&node->rightExpression);
}

void Interpreter2::visit(const ast::MultiselectListNode *node)
{
    // evaluate the multiselect list opration if the context doesn't holds null
    if (!getJsonValue(m_context2).is_null())
    {
        // create the array of results
        Json result(Json::value_t::array);
        // move the current context into a temporary variable in case it holds
        // a value, since the context member variable will get overwritten
        // during  the evaluation of sub expressions
        ContextValue contextValue {std::move(m_context2)};
        // iterate over the list of subexpressions
        for (auto& expression: node->expressions)
        {
            // assign a const lvalue ref to the context
            m_context2 = assignContextValue(getJsonValue(contextValue));
            // evaluate the subexpression
            visit(&expression);
            // copy the result of the subexpression into the list of results
            result.push_back(getJsonValue(m_context2));
        }
        // set the results of the projection
        m_context2 = std::move(result);
    }
}

void Interpreter2::visit(const ast::MultiselectHashNode *node)
{
    // evaluate the multiselect hash opration if the context doesn't holds null
    if (!getJsonValue(m_context2).is_null())
    {
        // create the array of results
        Json result(Json::value_t::object);
        // move the current context into a temporary variable in case it holds
        // a value, since the context member variable will get overwritten
        // during the evaluation of sub expressions
        ContextValue contextValue {std::move(m_context2)};
        // iterate over the list of subexpressions
        for (auto& keyValuePair: node->expressions)
        {
            // assign a const lvalue ref to the context
            m_context2 = assignContextValue(getJsonValue(contextValue));
            // evaluate the subexpression
            visit(&keyValuePair.second);
            // add a copy of the result of the sub expression as the value
            // for the key of the subexpression
            result[keyValuePair.first.identifier] = getJsonValue(m_context2);
        }
        // set the results of the projection
        m_context2 = std::move(result);
    }
}

void Interpreter2::visit(const ast::NotExpressionNode *node)
{
    // negate the result of the subexpression
    visit(&node->expression);
    m_context2 = !toBoolean(getJsonValue(m_context2));
}

void Interpreter2::visit(const ast::ComparatorExpressionNode *node)
{
    using Comparator = ast::ComparatorExpressionNode::Comparator;

    // thow an error if it's an unhandled operator
    if (node->comparator == Comparator::Unknown)
    {
        BOOST_THROW_EXCEPTION(InvalidAgrument{});
    }

    // move the current context into a temporary variable and use a const lvalue
    // reference as the context for evaluating the left side expression, so
    // the context can be reused for the evaluation of the right expression
    ContextValue contextValue {std::move(m_context2)};
    m_context2 = assignContextValue(getJsonValue(contextValue));
    // evaluate the left expression
    visit(&node->leftExpression);
    // move the left side results into a temporary variable
    ContextValue leftResultContext {std::move(m_context2)};
    const Json& leftResult = getJsonValue(leftResultContext);

    // set the context for the right side expression
    m_context2 = std::move(contextValue);
    // evaluate the right expression
    visit(&node->rightExpression);
    const Json& rightResult = getJsonValue(m_context2);

    if (node->comparator == Comparator::Equal)
    {
        m_context2 = leftResult == rightResult;
    }
    else if (node->comparator == Comparator::NotEqual)
    {
        m_context2 = leftResult != rightResult;
    }
    else
    {
        // if a non number is involved in an ordering comparison the result
        // should be null
        if (!leftResult.is_number() || !rightResult.is_number())
        {
            m_context2 = Json{};
        }
        else
        {
            if (node->comparator == Comparator::Less)
            {
                m_context2 = leftResult < rightResult;
            }
            else if (node->comparator == Comparator::LessOrEqual)
            {
                m_context2 = leftResult <= rightResult;
            }
            else if (node->comparator == Comparator::GreaterOrEqual)
            {
                m_context2 = leftResult >= rightResult;
            }
            else if (node->comparator == Comparator::Greater)
            {
                m_context2 = leftResult > rightResult;
            }
        }
    }
}

void Interpreter2::visit(const ast::OrExpressionNode *node)
{
    // evaluate the logic operator and return with the left side result
    // if it's equal to true
    evaluateLogicOperator(node, true);
}

void Interpreter2::visit(const ast::AndExpressionNode *node)
{
    // evaluate the logic operator and return with the left side result
    // if it's equal to false
    evaluateLogicOperator(node, false);
}

void Interpreter2::evaluateLogicOperator(
        const ast::BinaryExpressionNode* node,
        bool shortCircuitValue)
{
    // move the current context into a temporary variable and use a const lvalue
    // reference as the context for evaluating the left side expression, so
    // the context can be reused for the evaluation of the right expression
    ContextValue contextValue {std::move(m_context2)};
    m_context2 = assignContextValue(getJsonValue(contextValue));
    // evaluate the left expression
    visit(&node->leftExpression);
    // if the left side result is not enough for producing the final result
    if (toBoolean(getJsonValue(m_context2)) != shortCircuitValue)
    {
        // evaluate the right side expression
        m_context2 = std::move(contextValue);
        visit(&node->rightExpression);
    }
    else
    {
        // if the context of the logic operator holds a value but the
        // evaluation of the left side expression resulted in a const lvalue
        // ref then make a copy of the result, to avoid referring to a soon
        // to be destroyed value
        auto visitor = boost::hana::overload(
            [this](Json&, const JsonRef& leftResult) {
                m_context2 = leftResult.get();
            },
            [](auto, auto) {}
        );
        boost::apply_visitor(visitor, contextValue, m_context2);
    }
}

void Interpreter2::visit(const ast::ParenExpressionNode *node)
{
    // evaluate the sub expression
    visit(&node->expression);
}

void Interpreter2::visit(const ast::PipeExpressionNode *node)
{
    // evaluate the left followed by the right expression
    visit(&node->leftExpression);
    visit(&node->rightExpression);
}

void Interpreter2::visit(const ast::CurrentNode *)
{
}

void Interpreter2::visit(const ast::FilterExpressionNode *node)
{
    // move the current context into a temporary variable in case it holds
    // a value, since the context member variable will get overwritten during
    // the evaluation of the filter expression
    ContextValue contextValue {std::move(m_context2)};

    // visit the node with either an lvalue const ref or rvalue ref context
    boost::apply_visitor(ContextVisitor<ast::FilterExpressionNode>(this, node),
                         contextValue);
}

template <typename JsonT>
void Interpreter2::visit(const ast::FilterExpressionNode* node, JsonT&& context)
{
    // evaluate the filtering operation if the context holds an array
    if (context.is_array())
    {
        // create the array of results
        Json result(Json::value_t::array);
        // move or copy every item the context array which satisfies the
        // filtering condition
        std::copy_if(std::make_move_iterator(std::begin(context)),
                     std::make_move_iterator(std::end(context)),
                     std::back_inserter(result),
                     [this, &node](const Json& item)
        {
            // assign a const lvalue ref of the item to the context
            m_context2 = assignContextValue(item);
            // evaluate the filtering condition
            visit(&node->expression);
            // convert the result into a boolean
            return toBoolean(getJsonValue(m_context2));
        });

        // set the results of the projection
        m_context2 = std::move(result);
    }
    // otherwise evaluate to null
    else
    {
        m_context2 = {};
    }
}

void Interpreter2::visit(const ast::FunctionExpressionNode *node)
{
    auto it = m_functionMap.find(node->functionName);
    if (it == m_functionMap.end())
    {
        BOOST_THROW_EXCEPTION(UnknownFunction()
                              << InfoFunctionName(node->functionName));
    }

    const auto& descriptor = it->second;
    const auto& argumentArityValidator = std::get<0>(descriptor);
    const auto& function = std::get<1>(descriptor);
    if (!argumentArityValidator(node->arguments.size()))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentArity());
    }

    FunctionArgumentList argumentList = evaluateArguments(node->arguments);
    function(argumentList);
}

void Interpreter2::visit(const ast::ExpressionArgumentNode *)
{
}

Index Interpreter2::adjustSliceEndpoint(size_t length,
                                        Index endpoint,
                                        Index step) const
{
    if (endpoint < 0)
    {
        endpoint += length;
        if (endpoint < 0)
        {
            endpoint = step < 0 ? -1 : 0;
        }
    }
    else if (endpoint >= length)
    {
        endpoint = step < 0 ? length - 1: length;
    }
    return endpoint;
}

bool Interpreter2::toBoolean(const Json &json) const
{
    return json.is_number()
            || ((!json.is_boolean() || json.get<bool>())
                && (!json.is_string()
                    || !json.get_ptr<const std::string*>()->empty())
                && !json.empty());
}

Interpreter2::FunctionArgumentList
Interpreter2::evaluateArguments(const FunctionExpressionArgumentList &arguments)
{
    FunctionArgumentList argumentList;
    rng::transform(arguments, std::back_inserter(argumentList),
                   [this](auto argument)
    {
        FunctionArgument functionArgument;

        ast::ExpressionNode* expressionArgument
                = boost::get<ast::ExpressionNode>(&argument);
        if (expressionArgument)
        {
            auto context = m_context;
            this->visit(expressionArgument);
            functionArgument = std::move(m_context);
            m_context = std::move(context);
        }

        ast::ExpressionArgumentNode* expressionTypeArgument
                = boost::get<ast::ExpressionArgumentNode>(&argument);
        if (expressionTypeArgument)
        {
            functionArgument = expressionTypeArgument->expression;
        }

        return functionArgument;
    });
    return argumentList;
}

void Interpreter2::abs(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        m_context = std::abs(value->get<Json::number_integer_t>());
    }
    else
    {
        m_context = std::abs(value->get<Json::number_float_t>());
    }
}

void Interpreter2::avg(FunctionArgumentList &arguments)
{
    const Json* items = boost::get<Json>(&arguments[0]);
    if (items && items->is_array())
    {
        if (!items->empty())
        {
            double itemsSum = std::accumulate(items->cbegin(),
                                              items->cend(),
                                              0.0,
                                              [](double sum,
                                                 const Json& item) -> double
            {
                if (item.is_number_integer())
                {
                    return sum + item.get<Json::number_integer_t>();
                }
                else if (item.is_number_float())
                {
                    return sum + item.get<Json::number_float_t>();
                }
                else
                {
                    BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
                }
            });
            m_context = itemsSum / items->size();
        }
        else
        {
            m_context = Json{};
        }
    }
    else
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
}

void Interpreter2::contains(FunctionArgumentList &arguments)
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* item = boost::get<Json>(&arguments[1]);
    if (!subject || (!subject->is_array() && !subject->is_string()) || !item)
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    bool result = false;
    if (subject->is_array())
    {
        auto it = rng::find(*subject, *item);
        result = (it != std::end(*subject));
    }
    else if (subject->is_string())
    {
        auto stringSubject = subject->get_ptr<const String*>();
        auto stringItem = item->get_ptr<const String*>();
        result = boost::contains(*stringSubject, *stringItem);
    }
    m_context = result;
}

void Interpreter2::ceil(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        m_context = std::move(*value);
    }
    else
    {
        m_context = std::ceil(value->get<Json::number_float_t>());
    }
}

void Interpreter2::endsWith(FunctionArgumentList &arguments)
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* suffix = boost::get<Json>(&arguments[1]);
    if (!subject || !subject->is_string() || !suffix || !suffix->is_string())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    auto stringSubject = subject->get_ptr<const String*>();
    auto stringSuffix = suffix->get_ptr<const String*>();
    m_context = boost::ends_with(*stringSubject, *stringSuffix);
}

void Interpreter2::floor(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        m_context = std::move(*value);
    }
    else
    {
        m_context = std::floor(value->get<Json::number_float_t>());
    }
}

void Interpreter2::join(FunctionArgumentList &arguments)
{
    Json* glue = boost::get<Json>(&arguments[0]);
    const Json* array = boost::get<Json>(&arguments[1]);
    if (!glue || !glue->is_string() || !array || !array->is_array()
       || alg::any_of(*array, [](const auto& item) {return !item.is_string();}))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    std::vector<String> stringArray;
    rng::transform(*array, std::back_inserter(stringArray), [](const Json& item)
    {
        return item.get<String>();
    });
    m_context = alg::join(stringArray, glue->get<String>());
}

void Interpreter2::keys(FunctionArgumentList &arguments)
{
    const Json* object = boost::get<Json>(&arguments[0]);
    if (!object || !object->is_object())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    m_context = Json(Json::value_t::array);
    for (auto it = object->cbegin(); it != object->cend(); ++it)
    {
        m_context.push_back(it.key());
    }
}

void Interpreter2::length(FunctionArgumentList &arguments)
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    if (!subject || !(subject->is_array() || subject->is_object()
                      || subject->is_string()))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    if (subject->is_string())
    {
        const String* stringSubject = subject->get_ptr<const String*>();
        auto begin = UnicodeIteratorAdaptor(std::begin(*stringSubject));
        auto end = UnicodeIteratorAdaptor(std::end(*stringSubject));
        m_context = std::distance(begin, end);
    }
    else
    {
        m_context = subject->size();
    }
}

void Interpreter2::map(FunctionArgumentList &arguments)
{
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[0]);
    Json* array = boost::get<Json>(&arguments[1]);
    if (!expression || !array || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    rng::for_each(*array, [&](Json& item)
    {
        m_context = std::move(item);
        this->visit(expression);
        item = std::move(m_context);
    });
    m_context = std::move(*array);
}

void Interpreter2::merge(FunctionArgumentList &arguments)
{
    m_context = Json(Json::value_t::object);
    for (auto& argument: arguments)
    {
        Json* object = boost::get<Json>(&argument);
        if (!object || !object->is_object())
        {
            BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
        }

        if (m_context.empty())
        {
            m_context = std::move(*object);
        }
        else
        {
            for (auto it = std::begin(*object); it != std::end(*object); ++it)
            {
                m_context[it.key()] = std::move(*it);
            }
        }
    };
}

void Interpreter2::notNull(FunctionArgumentList &arguments)
{
    m_context = {};
    for (auto& argument: arguments)
    {
        Json* item = boost::get<Json>(&argument);
        if (!item)
        {
            BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
        }
        if (!item->is_null())
        {
            m_context = std::move(*item);
            break;
        }
    }
}

void Interpreter2::reverse(FunctionArgumentList &arguments)
{
    Json* subject = boost::get<Json>(&arguments[0]);
    if (!subject || !(subject->is_array() || subject->is_string()))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    if (subject->is_array())
    {
        rng::reverse(*subject);
    }
    else if (subject->is_string())
    {
        rng::reverse(*subject->get_ptr<String*>());
    }
    m_context = std::move(*subject);
}

void Interpreter2::sort(FunctionArgumentList &arguments)
{
    Json* array = boost::get<Json>(&arguments[0]);
    if (!array || !isComparableArray(*array))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    std::sort(std::begin(*array), std::end(*array));
    m_context = std::move(*array);
}

void Interpreter2::sortBy(FunctionArgumentList &arguments)
{
    Json* array = boost::get<Json>(&arguments[0]);
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[1]);
    if (!array || !expression || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    std::unordered_map<Json, Json> expressionResultsMap;
    auto firstItemType = Json::value_t::discarded;
    for (const auto& item: *array)
    {
        m_context = item;
        visit(expression);
        if (!(m_context.is_number() || m_context.is_string()))
        {
            BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
        }
        if (firstItemType == Json::value_t::discarded)
        {
            firstItemType = m_context.type();
        }
        else if (m_context.type() != firstItemType)
        {
            BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
        }
        expressionResultsMap[item] = std::move(m_context);
    }

    std::sort(std::begin(*array), std::end(*array),
              [&](auto& first, auto& second) -> bool
    {
        return expressionResultsMap[first] < expressionResultsMap[second];
    });
    m_context = std::move(*array);
}

void Interpreter2::startsWith(FunctionArgumentList &arguments)
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* prefix = boost::get<Json>(&arguments[1]);
    if (!subject || !subject->is_string() || !prefix || !prefix->is_string())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
    auto stringSubject = subject->get_ptr<const String*>();
    auto stringPrefix = prefix->get_ptr<const String*>();
    m_context = boost::starts_with(*stringSubject, *stringPrefix);
}

void Interpreter2::sum(FunctionArgumentList &arguments)
{
    const Json* items = boost::get<Json>(&arguments[0]);
    if (items && items->is_array())
    {
        double itemsSum = std::accumulate(items->cbegin(),
                                          items->cend(),
                                          0.0,
                                          [](double sum,
                                             const Json& item) -> double
        {
            if (item.is_number_integer())
            {
                return sum + item.get<Json::number_integer_t>();
            }
            else if (item.is_number_float())
            {
                return sum + item.get<Json::number_float_t>();
            }
            else
            {
                BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
            }
        });
        m_context = itemsSum;
    }
    else
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }
}

void Interpreter2::toArray(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    if (value->is_array())
    {
        m_context = std::move(*value);
    }
    else
    {
        m_context = {};
        m_context.push_back(std::move(*value));
    }
}

void Interpreter2::toString(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    if (value->is_string())
    {
        m_context = std::move(*value);
    }
    else
    {
        m_context = value->dump();
    }
}

void Interpreter2::toNumber(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    m_context = {};
    if (value->is_number())
    {
        m_context = std::move(*value);
    }
    else if (value->is_string())
    {
        try
        {
            m_context = std::stod(*value->get_ptr<const String*>());
        }
        catch (std::exception&)
        {
        }
    }
}

void Interpreter2::type(FunctionArgumentList &arguments)
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    String result;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    switch (value->type())
    {
    case Json::value_t::number_float:
    case Json::value_t::number_unsigned:
    case Json::value_t::number_integer: result = "number"; break;
    case Json::value_t::string: result = "string"; break;
    case Json::value_t::boolean: result = "boolean"; break;
    case Json::value_t::array: result = "array"; break;
    case Json::value_t::object: result = "object"; break;
    case Json::value_t::discarded:
    case Json::value_t::null:
    default: result = "null";
    }
#pragma clang diagnostic pop
    m_context = std::move(result);
}

void Interpreter2::values(FunctionArgumentList &arguments)
{
    const Json* object = boost::get<Json>(&arguments[0]);
    if (!object || !object->is_object())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    m_context = {};
    for (auto it = object->begin(); it != object->end(); ++it)
    {
        m_context.push_back(std::move(it.value()));
    }
}

void Interpreter2::max(FunctionArgumentList &arguments,
                              const JsonComparator &comparator)
{
    const Json* array = boost::get<Json>(&arguments[0]);
    if (!array || !isComparableArray(*array))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    m_context = {};
    auto it = rng::max_element(*array, comparator);
    if (it != array->end())
    {
        m_context = std::move(*it);
    }
}

void Interpreter2::maxBy(FunctionArgumentList &arguments,
                                       const JsonComparator &comparator)
{
    Json* array = boost::get<Json>(&arguments[0]);
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[1]);
    if (!array || !expression || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    Json expressionResults(Json::value_t::array);
    rng::transform(*array, std::back_inserter(expressionResults),
                   [&](const Json& item)
    {
        m_context = item;
        this->visit(expression);
        if (!(m_context.is_number() || m_context.is_string()))
        {
            BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
        }
        return m_context;
    });
    auto maxResultsIt = rng::max_element(expressionResults, comparator);
    auto maxIt = std::begin(*array)
            + std::distance(std::begin(expressionResults), maxResultsIt);
    m_context = std::move(*maxIt);
}

bool Interpreter2::isComparableArray(const Json &array) const
{
    bool result = false;
    auto notComparablePredicate = [](const auto& item, const auto& firstItem)
    {
        return !((item.is_number() && firstItem.is_number())
                 || (item.is_string() && firstItem.is_string()));
    };
    if (array.is_array())
    {
        result = true;
        if (!array.empty())
        {
            result = !alg::any_of(array, std::bind(notComparablePredicate,
                                                   std::placeholders::_1,
                                                   std::cref(array[0])));
        }
    }
    return result;
}
}} // namespace jmespath::interpreter

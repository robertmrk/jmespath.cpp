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
#include "jmespath/interpreter/expressionevaluator.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/detail/exceptions.h"
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

namespace jmespath { namespace interpreter {

namespace rng = boost::range;

ExpressionEvaluator::ExpressionEvaluator()
    : AbstractVisitor()
{
}

ExpressionEvaluator::ExpressionEvaluator(const Json &contextValue)
    : AbstractVisitor()
{
    setContext(contextValue);
}

void ExpressionEvaluator::setContext(const Json &value)
{
    m_context = value;
}

Json ExpressionEvaluator::currentContext() const
{
    return m_context;
}

void ExpressionEvaluator::evaluateProjection(ast::ExpressionNode *expression)
{
    Json contextArray = m_context;
    Json result;
    if (contextArray.is_array())
    {
        result = Json(Json::value_t::array);
        for (auto item: contextArray)
        {
            m_context = item;
            visit(expression);
            if (!m_context.is_null())
            {
                result.push_back(m_context);
            }
        }
    }
    m_context = result;
}

void ExpressionEvaluator::visit(ast::AbstractNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::ExpressionNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::IdentifierNode *node)
{
    Json result;
    if (m_context.is_object())
    {
        result = m_context[node->identifier];
    }
    m_context = result;
}

void ExpressionEvaluator::visit(ast::RawStringNode *node)
{
    m_context = node->rawString;
}

void ExpressionEvaluator::visit(ast::LiteralNode *node)
{
    m_context = Json::parse(node->literal);
}

void ExpressionEvaluator::visit(ast::SubexpressionNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::IndexExpressionNode *node)
{
    visit(&node->leftExpression);
    if (m_context.is_array())
    {
        visit(&node->bracketSpecifier);
        if (node->isProjection())
        {
            evaluateProjection(&node->rightExpression);
        }
    }
    else
    {
        m_context = {};
    }
}

void ExpressionEvaluator::visit(ast::ArrayItemNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        int arrayIndex = node->index;
        if (arrayIndex < 0)
        {
            arrayIndex = m_context.size() + arrayIndex;
        }
        if ((arrayIndex >= 0 ) && (arrayIndex < m_context.size()))
        {
            result = m_context[arrayIndex];
        }
    }
    m_context = result;
}

void ExpressionEvaluator::visit(ast::FlattenOperatorNode *node)
{
    Json result;
    Json contextArray = m_context;
    if (contextArray.is_array())
    {
        Json arrayValue(Json::value_t::array);
        for (auto const& item: contextArray)
        {
            if (item.is_array())
            {
                rng::copy(item, std::back_inserter(arrayValue));
            }
            else
            {
                arrayValue.push_back(item);
            }
        }
        result = arrayValue;
    }
    m_context = result;
}

void ExpressionEvaluator::visit(ast::BracketSpecifierNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::SliceExpressionNode *node)
{
    Json result;
    Json contextArray = m_context;
    if (contextArray.is_array())
    {
        int startIndex = 0;
        int endIndex = 0;
        int step = 1;
        int length = contextArray.size();

        if (node->step)
        {
            if (*node->step == 0)
            {
                BOOST_THROW_EXCEPTION(detail::InvalidValue{});
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
            endIndex = step < 0 ? -1 : length;
        }
        else
        {
            endIndex = adjustSliceEndpoint(length, *node->stop, step);
        }

        result = Json(Json::value_t::array);
        auto beginIt = std::begin(contextArray);
        auto it = beginIt + startIndex;
        auto stopIt = beginIt + endIndex;

        while (((step > 0) && (it < stopIt))
               || ((step < 0) && (it > stopIt)))
        {
            result.push_back(*it);
            it += step;
        }
    }
    m_context = result;
}

void ExpressionEvaluator::visit(ast::ListWildcardNode *node)
{
    if (!m_context.is_array())
    {
        m_context = {};
    }
}

void ExpressionEvaluator::visit(ast::HashWildcardNode *node)
{
    visit(&node->leftExpression);
    Json result;
    if (m_context.is_object())
    {
        rng::copy(m_context, std::back_inserter(result));
    }
    m_context = result;
    evaluateProjection(&node->rightExpression);
}

void ExpressionEvaluator::visit(ast::MultiselectListNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::array);
        Json childContext = m_context;
        for (auto& expression: node->expressions)
        {
            visit(&expression);
            result.push_back(m_context);
            m_context = childContext;
        }
        m_context = result;
    }
}

void ExpressionEvaluator::visit(ast::MultiselectHashNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::object);
        Json childContext = m_context;
        for (auto& keyValuePair: node->expressions)
        {
            visit(&keyValuePair.second);
            result[keyValuePair.first.identifier] = m_context;
            m_context = childContext;
        }
        m_context = result;
    }
}

void ExpressionEvaluator::visit(ast::NotExpressionNode *node)
{
    visit(&node->expression);
    bool result = !toBoolean(m_context);
    m_context = Json(Json::value_t::boolean);
    m_context = result;
}

void ExpressionEvaluator::visit(ast::ComparatorExpressionNode *node)
{
    using Comparator = ast::ComparatorExpressionNode::Comparator;

    if (node->comparator == Comparator::Unknown)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidAgrument{});
    }

    Json childContext = m_context;
    visit(&node->leftExpression);
    Json leftResult = m_context;

    m_context = childContext;
    visit(&node->rightExpression);
    Json rightResult = m_context;

    m_context = Json(Json::value_t::boolean);
    if (node->comparator == Comparator::Less)
    {
        m_context = leftResult < rightResult;
    }
    else if (node->comparator == Comparator::LessOrEqual)
    {
        m_context = leftResult <= rightResult;
    }
    else if (node->comparator == Comparator::Equal)
    {
        m_context = leftResult == rightResult;
    }
    else if (node->comparator == Comparator::GreaterOrEqual)
    {
        m_context = leftResult >= rightResult;
    }
    else if (node->comparator == Comparator::Greater)
    {
        m_context = leftResult > rightResult;
    }
    else if (node->comparator == Comparator::NotEqual)
    {
        m_context = leftResult != rightResult;
    }
}

void ExpressionEvaluator::visit(ast::OrExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (!toBoolean(m_context))
    {
        m_context = childContext;
        visit(&node->rightExpression);
    }
}

void ExpressionEvaluator::visit(ast::AndExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (toBoolean(m_context))
    {
        m_context = childContext;
        visit(&node->rightExpression);
    }
}

void ExpressionEvaluator::visit(ast::ParenExpressionNode *node)
{
    visit(&node->expression);
}

void ExpressionEvaluator::visit(ast::PipeExpressionNode *node)
{
    visit(&node->leftExpression);
    visit(&node->rightExpression);
}

int ExpressionEvaluator::adjustSliceEndpoint(int length,
                                             int endpoint,
                                             int step) const
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

bool ExpressionEvaluator::toBoolean(const Json &json) const
{
    return json.is_number()
            || (!json.is_null()
                && (!json.is_boolean() || (json.get<bool>() != false))
                && (!json.is_string() || !json.get<std::string>().empty())
                && ((!json.is_array() && !json.is_object()) || !json.empty()));
}
}} // namespace jmespath::interpreter

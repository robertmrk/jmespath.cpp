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
#include "src/interpreter/interpreter.h"
#include "src/ast/allnodes.h"
#include "jmespath/exceptions.h"
#include <numeric>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string.hpp>

namespace jmespath { namespace interpreter {

namespace rng = boost::range;
namespace alg = boost::algorithm;

Interpreter::Interpreter(const Json &value)
    : AbstractVisitor()
{
    setContext(value);
    using std::placeholders::_1;
    using std::bind;
    auto exactlyOne = bind(std::equal_to<size_t>{}, _1, 1);
    auto exactlyTwo = bind(std::equal_to<size_t>{}, _1, 2);
    auto zeroOrMore = bind(std::greater_equal<size_t>{}, _1, 0);
    auto oneOrMore = bind(std::greater_equal<size_t>{}, _1, 1);
    m_functionMap = {
        {"abs", {exactlyOne, bind(&Interpreter::abs, this, _1)}},
        {"avg", {exactlyOne, bind(&Interpreter::avg, this, _1)}},
        {"contains", {exactlyTwo, bind(&Interpreter::contains, this, _1)}},
        {"ceil", {exactlyOne, bind(&Interpreter::ceil, this, _1)}},
        {"ends_with", {exactlyTwo, bind(&Interpreter::endsWith, this, _1)}},
        {"floor", {exactlyOne, bind(&Interpreter::floor, this, _1)}},
        {"join", {exactlyTwo, bind(&Interpreter::join, this, _1)}},
        {"keys", {exactlyOne, bind(&Interpreter::keys, this, _1)}},
        {"length", {exactlyOne, bind(&Interpreter::length, this, _1)}},
        {"map", {exactlyTwo, bind(&Interpreter::map, this, _1)}},
        {"max", {exactlyOne,
                 bind(&Interpreter::max, this, _1, std::less<Json>{})}},
        {"max_by", {exactlyTwo,
                    bind(&Interpreter::maxBy, this, _1, std::less<Json>{})}},
        {"merge", {zeroOrMore, bind(&Interpreter::merge, this, _1)}},
        {"min", {exactlyOne,
                 bind(&Interpreter::max, this, _1, std::greater<Json>{})}},
        {"min_by", {exactlyTwo,
                    bind(&Interpreter::maxBy, this, _1, std::greater<Json>{})}},
        {"not_null", {oneOrMore, bind(&Interpreter::notNull, this, _1)}},
        {"reverse", {exactlyOne, bind(&Interpreter::reverse, this, _1)}},
        {"sort", {exactlyOne, bind(&Interpreter::sort, this, _1)}},
        {"sort_by", {exactlyTwo, bind(&Interpreter::sortBy, this, _1)}},
        {"starts_with", {exactlyTwo, bind(&Interpreter::startsWith, this, _1)}},
        {"sum", {exactlyOne, bind(&Interpreter::sum, this, _1)}},
        {"to_array", {exactlyOne, bind(&Interpreter::toArray, this, _1)}},
        {"to_string", {exactlyOne, bind(&Interpreter::toString, this, _1)}},
        {"to_number", {exactlyOne, bind(&Interpreter::toNumber, this, _1)}},
        {"type", {exactlyOne, bind(&Interpreter::type, this, _1)}},
        {"values", {exactlyOne, bind(&Interpreter::values, this, _1)}}
    };
}

void Interpreter::setContext(const Json &value)
{
    m_context = value;
}

Json Interpreter::currentContext() const
{
    return m_context;
}

void Interpreter::evaluateProjection(const ast::ExpressionNode *expression)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        Json contextArray = std::move(m_context);
        for (auto& item: contextArray)
        {
            m_context = std::move(item);
            visit(expression);
            if (!m_context.is_null())
            {
                result.push_back(std::move(m_context));
            }
        }
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::AbstractNode *node)
{
    node->accept(this);
}

void Interpreter::visit(const ast::ExpressionNode *node)
{
    node->accept(this);
}

void Interpreter::visit(const ast::IdentifierNode *node)
{
    Json result;
    if (m_context.is_object())
    {
        result = std::move(m_context[node->identifier]);
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::RawStringNode *node)
{
    m_context = node->rawString;
}

void Interpreter::visit(const ast::LiteralNode *node)
{
    m_context = Json::parse(node->literal);
}

void Interpreter::visit(const ast::SubexpressionNode *node)
{
    node->accept(this);
}

void Interpreter::visit(const ast::IndexExpressionNode *node)
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

void Interpreter::visit(const ast::ArrayItemNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        auto arrayIndex = node->index;
        if (arrayIndex < 0)
        {
            arrayIndex += m_context.size();
        }
        if ((arrayIndex >= 0) && (arrayIndex < m_context.size()))
        {
            result = std::move(m_context[static_cast<size_t>(arrayIndex)]);
        }
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::FlattenOperatorNode *)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        for (auto& item: m_context)
        {
            if (item.is_array())
            {
                std::move(std::begin(item),
                          std::end(item),
                          std::back_inserter(result));
            }
            else
            {
                result.push_back(std::move(item));
            }
        }
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::BracketSpecifierNode *node)
{
    node->accept(this);
}

void Interpreter::visit(const ast::SliceExpressionNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        Index startIndex = 0;
        Index stopIndex = 0;
        Index step = 1;
        size_t length = m_context.size();

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

        result = Json(Json::value_t::array);
        for (auto i = startIndex;
             step > 0 ? (i < stopIndex) : (i > stopIndex);
             i += step)
        {
            size_t arrayIndex = static_cast<size_t>(i);
            result.push_back(std::move(m_context[arrayIndex]));
        }
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::ListWildcardNode*)
{
    if (!m_context.is_array())
    {
        m_context = {};
    }
}

void Interpreter::visit(const ast::HashWildcardNode *node)
{
    visit(&node->leftExpression);
    Json result;
    if (m_context.is_object())
    {
        std::move(std::begin(m_context),
                  std::end(m_context),
                  std::back_inserter(result));
    }
    m_context = std::move(result);
    evaluateProjection(&node->rightExpression);
}

void Interpreter::visit(const ast::MultiselectListNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::array);
        Json childContext = std::move(m_context);
        for (auto& expression: node->expressions)
        {
            m_context = childContext;
            visit(&expression);
            result.push_back(std::move(m_context));
        }
        m_context = std::move(result);
    }
}

void Interpreter::visit(const ast::MultiselectHashNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::object);
        Json childContext = std::move(m_context);
        for (auto& keyValuePair: node->expressions)
        {
            m_context = childContext;
            visit(&keyValuePair.second);
            result[keyValuePair.first.identifier] = std::move(m_context);
        }
        m_context = std::move(result);
    }
}

void Interpreter::visit(const ast::NotExpressionNode *node)
{
    visit(&node->expression);
    m_context = !toBoolean(m_context);
}

void Interpreter::visit(const ast::ComparatorExpressionNode *node)
{
    using Comparator = ast::ComparatorExpressionNode::Comparator;

    if (node->comparator == Comparator::Unknown)
    {
        BOOST_THROW_EXCEPTION(InvalidAgrument{});
    }

    Json childContext = m_context;
    visit(&node->leftExpression);
    Json leftResult = std::move(m_context);

    m_context = std::move(childContext);
    visit(&node->rightExpression);
    Json rightResult = std::move(m_context);

    if (node->comparator == Comparator::Equal)
    {
        m_context = leftResult == rightResult;
    }
    else if (node->comparator == Comparator::NotEqual)
    {
        m_context = leftResult != rightResult;
    }
    else
    {
        // if a non number is involved in an ordering comparison the result
        // should be null
        if (!leftResult.is_number() || !rightResult.is_number())
        {
            m_context = Json{};
        }
        else
        {
            if (node->comparator == Comparator::Less)
            {
                m_context = leftResult < rightResult;
            }
            else if (node->comparator == Comparator::LessOrEqual)
            {
                m_context = leftResult <= rightResult;
            }
            else if (node->comparator == Comparator::GreaterOrEqual)
            {
                m_context = leftResult >= rightResult;
            }
            else if (node->comparator == Comparator::Greater)
            {
                m_context = leftResult > rightResult;
            }
        }
    }
}

void Interpreter::visit(const ast::OrExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (!toBoolean(m_context))
    {
        m_context = std::move(childContext);
        visit(&node->rightExpression);
    }
}

void Interpreter::visit(const ast::AndExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (toBoolean(m_context))
    {
        m_context = std::move(childContext);
        visit(&node->rightExpression);
    }
}

void Interpreter::visit(const ast::ParenExpressionNode *node)
{
    visit(&node->expression);
}

void Interpreter::visit(const ast::PipeExpressionNode *node)
{
    visit(&node->leftExpression);
    visit(&node->rightExpression);
}

void Interpreter::visit(const ast::CurrentNode *)
{
}

void Interpreter::visit(const ast::FilterExpressionNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        Json contextArray = std::move(m_context);
        std::copy_if(std::make_move_iterator(std::begin(contextArray)),
                     std::make_move_iterator(std::end(contextArray)),
                     std::back_inserter(result),
                     [&](const auto& item)
        {
            m_context = item;
            this->visit(&node->expression);
            return this->toBoolean(m_context);
        });
    }
    m_context = std::move(result);
}

void Interpreter::visit(const ast::FunctionExpressionNode *node)
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

void Interpreter::visit(const ast::ExpressionArgumentNode *)
{
}

Index Interpreter::adjustSliceEndpoint(size_t length,
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

bool Interpreter::toBoolean(const Json &json) const
{
    return json.is_number()
            || ((!json.is_boolean() || json.get<bool>())
                && (!json.is_string()
                    || !json.get_ptr<const std::string*>()->empty())
                && !json.empty());
}

Interpreter::FunctionArgumentList
Interpreter::evaluateArguments(const FunctionExpressionArgumentList &arguments)
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

void Interpreter::abs(FunctionArgumentList &arguments)
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

void Interpreter::avg(FunctionArgumentList &arguments)
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

void Interpreter::contains(FunctionArgumentList &arguments)
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

void Interpreter::ceil(FunctionArgumentList &arguments)
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

void Interpreter::endsWith(FunctionArgumentList &arguments)
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

void Interpreter::floor(FunctionArgumentList &arguments)
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

void Interpreter::join(FunctionArgumentList &arguments)
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

void Interpreter::keys(FunctionArgumentList &arguments)
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

void Interpreter::length(FunctionArgumentList &arguments)
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

void Interpreter::map(FunctionArgumentList &arguments)
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

void Interpreter::merge(FunctionArgumentList &arguments)
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

void Interpreter::notNull(FunctionArgumentList &arguments)
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

void Interpreter::reverse(FunctionArgumentList &arguments)
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

void Interpreter::sort(FunctionArgumentList &arguments)
{
    Json* array = boost::get<Json>(&arguments[0]);
    if (!array || !isComparableArray(*array))
    {
        BOOST_THROW_EXCEPTION(InvalidFunctionArgumentType());
    }

    std::sort(std::begin(*array), std::end(*array));
    m_context = std::move(*array);
}

void Interpreter::sortBy(FunctionArgumentList &arguments)
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

void Interpreter::startsWith(FunctionArgumentList &arguments)
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

void Interpreter::sum(FunctionArgumentList &arguments)
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

void Interpreter::toArray(FunctionArgumentList &arguments)
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

void Interpreter::toString(FunctionArgumentList &arguments)
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

void Interpreter::toNumber(FunctionArgumentList &arguments)
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

void Interpreter::type(FunctionArgumentList &arguments)
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

void Interpreter::values(FunctionArgumentList &arguments)
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

void Interpreter::max(FunctionArgumentList &arguments,
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

void Interpreter::maxBy(FunctionArgumentList &arguments,
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

bool Interpreter::isComparableArray(const Json &array) const
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
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
#ifndef NODERANK_H
#define NODERANK_H
#include "jmespath/ast/allnodes.h"

namespace jmespath { namespace parser {

namespace {

/**
 * @brief Returns the rank of the given @a node object's type.
 * @param node The node object that should be ranked.
 * @tparam T The type of the @a node object.
 * @return Returns the rank of the node as an integer.
 */
template <typename T>
int nodeRank(const T& node)
{
    return 0;
}
/**
 * @brief The NodeRankVisitor struct is a functor which returns the rank of a
 * node in a variant
 */
struct NodeRankVisitor : public boost::static_visitor<>
{
    /**
     * The type of the result.
     */
    using result_type = int;
    /**
     * Calls the nodeRank function on the given @a node.
     * @param node The node object that should be ranked.
     * @tparam T The type of the @a node object.
     */
    template <typename T>
    int operator()(const T& node) const
    {
        return nodeRank(node);
    }
};

template <>
int nodeRank(const ast::ExpressionNode& node)
{
    if(node.isNull())
    {
        return -1;
    }
    else
    {
        return boost::apply_visitor(NodeRankVisitor{}, node.value);
    }
}

template <>
int nodeRank(const ast::SubexpressionNode&)
{
    return 1;
}

template <>
int nodeRank(const ast::BracketSpecifierNode& node)
{
    return boost::apply_visitor(NodeRankVisitor{}, node.value);
}

template <>
int nodeRank(const ast::IndexExpressionNode& node)
{
    return nodeRank(node.bracketSpecifier);
}

template <>
int nodeRank(const ast::ArrayItemNode& node)
{
    return 1;
}

template <>
int nodeRank(const ast::FlattenOperatorNode& node)
{
    return 2;
}

template <>
int nodeRank(const ast::SliceExpressionNode& node)
{
    return 2;
}
} // anonymous namespace
}} // namespace jmespath::parser
#endif // NODERANK_H

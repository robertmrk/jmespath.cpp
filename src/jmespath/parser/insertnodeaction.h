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
#ifndef INSERTBINARYEXPRESSIONNODEACTION_H
#define INSERTBINARYEXPRESSIONNODEACTION_H
#include "jmespath/parser/noderank.h"
#include "jmespath/ast/allnodes.h"
#include <boost/variant/polymorphic_get.hpp>
#include <boost/optional.hpp>

namespace jmespath { namespace parser {

/**
 * @brief The InsertNodeAction class is a functor for inserting the given
 * @a node into the AST whose root node is specified with @a targetNode.
 *
 * The functor iterates over a sequence of nodes and if @a NodeInsertConditionT
 * returns true for the given combination of @a targetNode and @a node the
 * @a node will be inserted using @a NodeInserterT. If @a NodeInsertConditionT
 * returns false it will try to insert @a node at the next location supplied by
 * @a ChildExtractorT.
 * @tparam ChildExtractorT
 * @tparam NodeInserterT
 * @tparam NodeInsertConditionT
 */
template <typename ChildExtractorT,
          typename NodeInserterT,
          typename NodeInsertConditionT>
class InsertNodeAction
{
public:
    /**
     * The action's result type
     */
    using result_type = void;

    /**
     * @brief Inserts the given @a node into the AST whose root node is
     * specified with @a targetNode.
     * @param targetNode The root node of the AST.
     * @param node The node which should be inserted.
     * @{
     */
    template <typename T>
    void operator()(ast::ExpressionNode& targetNode, T& node) const
    {
        (*this)(&targetNode, node);
    }

    template <typename T>
    void operator()(ast::ExpressionNode* targetNode, T& node) const
    {
        if (targetNode)
        {
            if (m_insertCondition(targetNode, &node))
            {
                m_nodeInserter(targetNode, &node);
            }
            else
            {
                (*this)(m_childExtractor(targetNode), node);
            }
        }
    }
    /** @}*/
private:
    ChildExtractorT m_childExtractor;
    NodeInserterT m_nodeInserter;
    NodeInsertConditionT m_insertCondition;
};
}} // namespace jmespath::parser
#endif // INSERTBINARYEXPRESSIONNODEACTION_H

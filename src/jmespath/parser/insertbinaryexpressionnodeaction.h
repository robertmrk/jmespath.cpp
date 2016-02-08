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

struct InsertBinaryExpressionNodeAction
{
    /**
     * The actions return type
     */
    using return_type = void;
    /**
     * @brief Inserts @a currentNode into its apropriate position inside the
     * AST based on its rank.
     * @param targetNode Reference to a node in the AST.
     * @param currentNode The node that should be inserted.
     * @param firstExpression An optional reference to the leftmost terminal
     * node. If specified, it will be inserted as the left expression of the
     * leftmost binary node.
     * @tparam T The type of @a currentNode.
     */
    template <typename T>
    void operator()(ast::ExpressionNode& targetNode,
                    T& currentNode,
                    boost::optional<ast::ExpressionNode&> firstExpression
                    = {}) const
    {
        auto targetBinaryNode = toBinaryNode(&targetNode);
        // if the root node is not a binary node replace it with the current
        // node
        if (!targetBinaryNode)
        {
            targetNode.value = currentNode;
        }
        else
        {
            int targetNodeRank = nodeRank(targetNode);
            int currentNodeRank = nodeRank(currentNode);
            // if current node has a higher rank then target node, or if their
            // ranks are equal and current node is a projection and target node
            // doesn't terminates projections
            if ((currentNodeRank > targetNodeRank)
                     || ((currentNodeRank == targetNodeRank)
                         && (currentNode.isProjection()
                             && ! targetBinaryNode->stopsProjection())))
            {
                // set target node as current node's right expression and
                // replace the target node
                currentNode.rightExpression = targetNode;
                targetNode = currentNode;
            }
            else
            {
                // insert current node into the left node of the target
                (*this)(targetBinaryNode->leftExpression,
                        currentNode,
                        firstExpression);
            }
        }
        // if the firstExpression argument was specified
        if (firstExpression)
        {
            // insert it to the left of the leftmost binary node
            ast::BinaryExpressionNode* leftmostNode
                = leftmostBinaryNode(&targetNode, nodeRank(*firstExpression));
            if (leftmostNode)
            {
                leftmostNode->leftExpression = *firstExpression;
            }
        }
    }
    /**
     * @brief Casts the given @a node to a BinaryExpressionNode if the node
     * contained in the ExpressionNode inherits from BinaryExpressionNode.
     * @param node Pointer to an ExpressionNode object.
     * @return Returns a BinaryExpressionNode pointer if the casting was
     * successful otherwise returns a nullptr.
     */
    ast::BinaryExpressionNode* toBinaryNode(ast::ExpressionNode* node) const
    {
        return boost::polymorphic_get<ast::BinaryExpressionNode>(
                    &node->value);
    }
    /**
     * @brief Finds the leftmost binary node under the given @a node whose rank
     * is larger or equal to @a minimumRank.
     * @param node The root node of the subtree where the find operation should
     * be executed.
     * @param minimumRank Specifies the rank value under which no nodes should
     * be considered.
     * @return Returns a pointer to the leftmost binary node or a nullptr if no
     * binary node was found under node which has at least the rank of
     * @a minimumRank.
     */
    ast::BinaryExpressionNode* leftmostBinaryNode(ast::ExpressionNode* node,
                                                  int minimumRank) const
    {
        ast::BinaryExpressionNode* binaryNode = nullptr;
        // if node is a valid pointer
        if (node)
        {
            // get the rank of the node
            int currentNodeRank = nodeRank(*node);
            // if the current node's rank is higher or equal to minimum rank
            if (currentNodeRank >= minimumRank)
            {
                // get a pointer to the binary node
                binaryNode = toBinaryNode(node);
                // if the pointer is valid
                if (binaryNode)
                {
                    // check the left node of the current node for better
                    // matches
                    ast::BinaryExpressionNode* leftNode
                            = leftmostBinaryNode(&binaryNode->leftExpression,
                                                 minimumRank);
                    // if a valid node was found to the left then return it as
                    // the match
                    if (leftNode)
                    {
                        binaryNode = leftNode;
                    }
                }
            }
        }
        return binaryNode;
    }
};
}} // namespace jmespath::parser
#endif // INSERTBINARYEXPRESSIONNODEACTION_H

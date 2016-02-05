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
#ifndef BRACKETSPECIFIERNODE_H
#define BRACKETSPECIFIERNODE_H
#include "jmespath/ast/variantnode.h"
#include <boost/fusion/include/adapt_struct.hpp>

namespace jmespath { namespace ast {

class ArrayItemNode;
class FlattenOperatorNode;
/**
 * @brief The BracketSpecifierNode class represents a JMESPath bracket
 * specifier.
 */
class BracketSpecifierNode : public AbstractNode
{
public:
    using Expression = VariantNode<boost::recursive_wrapper<ArrayItemNode>,
        boost::recursive_wrapper<FlattenOperatorNode> >;
    /**
     * @brief Constructs an empty BracketSpecifierNode object.
     */
    BracketSpecifierNode();
    /**
     * @brief Constructs a BracketSpecifierNode object with the given
     * @a expression as its value.
     * @param expression The node's child expression.
     */
    BracketSpecifierNode(const Expression& expression);
    /**
     * @brief Calls the visit method of the given \a visitor with the
     * dynamic type of the node.
     * @param visitor A visitor implementation
     */
    void accept(interpreter::AbstractVisitor *visitor) override;
    /**
     * @brief Equality compares this node to the \a other
     * @param other The node that should be compared.
     * @return Returns true if this object is equal to the \a other, otherwise
     * false
     */
    bool operator==(const BracketSpecifierNode& other) const;
    /**
     * @brief Returns whather this expression requires the projection of
     * subsequent expressions.
     * @return Returns true if projection is required, otherwise returns false.
     */
    bool isProjection() const;
    /**
     * @brief The node's child expression
     */
    Expression expression;
};
}} // namespace jmespath::ast

BOOST_FUSION_ADAPT_STRUCT(
    jmespath::ast::BracketSpecifierNode,
    (jmespath::ast::BracketSpecifierNode::Expression, expression)
)
#endif // BRACKETSPECIFIERNODE_H

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
#ifndef ABSTRACTVISITOR_H
#define ABSTRACTVISITOR_H

namespace jmespath { namespace ast {

class AbstractNode;
class Node;
class ExpressionNode;
class IdentifierNode;
class RawStringNode;
class LiteralNode;
class SubexpressionNode;
class IndexExpressionNode;
class ArrayItemNode;
class FlattenOperatorNode;
class BracketSpecifierNode;
class SliceExpressionNode;
class ListWildcardNode;
class HashWildcardNode;
class MultiselectListNode;
class MultiselectHashNode;
class NotExpressionNode;
class ComparatorExpressionNode;
}} // namespace jmespath::ast

/**
 * @namespace jmespath::interpreter
 * @brief Classes for interpreting the AST of the JMESPath expression.
 */
namespace jmespath { namespace interpreter {
/**
 * @brief The AbstractVisitor class is an interface which
 * defines the member functions required to visit every
 * type of AST node
 */
class AbstractVisitor
{
public:
    /**
     * @brief Visits the given @a node.
     * @param node Pointer to the node
     * @{
     */
    virtual void visit(ast::AbstractNode* node) = 0;
    virtual void visit(ast::ExpressionNode* node) = 0;
    virtual void visit(ast::IdentifierNode* node) = 0;
    virtual void visit(ast::RawStringNode* node) = 0;
    virtual void visit(ast::LiteralNode* node) = 0;
    virtual void visit(ast::SubexpressionNode* node) = 0;
    virtual void visit(ast::IndexExpressionNode* node) = 0;
    virtual void visit(ast::ArrayItemNode* node) = 0;
    virtual void visit(ast::FlattenOperatorNode* node) = 0;
    virtual void visit(ast::BracketSpecifierNode* node) = 0;
    virtual void visit(ast::SliceExpressionNode* node) = 0;    
    virtual void visit(ast::ListWildcardNode* node) = 0;
    virtual void visit(ast::HashWildcardNode* node) = 0;
    virtual void visit(ast::MultiselectListNode* node) = 0;
    virtual void visit(ast::MultiselectHashNode* node) = 0;
    virtual void visit(ast::NotExpressionNode* node) = 0;
    virtual void visit(ast::ComparatorExpressionNode* node) = 0;
    /** @}*/
};
}} // namespace jmespath::interpreter
#endif // ABSTRACTVISITOR_H

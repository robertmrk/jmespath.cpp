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
#ifndef EXPRESSIONEVALUATOR_H
#define EXPRESSIONEVALUATOR_H
#include "jmespath/interpreter/abstractvisitor.h"
#include "jmespath/detail/types.h"

namespace jmespath { namespace interpreter {

using detail::Json;
/**
 * @brief The ExpressionEvaluator class evaluates the AST structure.
 */
class ExpressionEvaluator : public AbstractVisitor
{
public:
    /**
     * @brief Constructs an empty ExpressionEvaluator object.
     *
     * Before calling any of the visit functions the object should be
     * initialized by setting the evaluation context with the \sa setContext
     * method.
     */
    ExpressionEvaluator();
    /**
     * @brief Constructs an ExpressionEvaluator object with the given
     * @a document as the context for the evaluation of the AST.
     * @param document JSON document on which the AST will be evaluated
     */
    ExpressionEvaluator(const Json& contextValue);
    /**
     * @brief Sets the context of the evaluation.
     * @param value JSON document to be used as the context.
     */
    void setContext(const Json& value);
    /**
     * @brief Returns the current evaluation context.
     * @return JSON document used as the context.
     */
    Json currentContext() const;

    void visit(ast::AbstractNode *node) override;
    void visit(ast::Node* node) override;
    void visit(ast::ExpressionNode *node) override;
    void visit(ast::IdentifierNode *node) override;
    void visit(ast::RawStringNode *node) override;
    void visit(ast::LiteralNode* node) override;
    void visit(ast::SubexpressionNode* node) override;
    void visit(ast::IndexExpressionNode* node) override;
    void visit(ast::ArrayItemNode* node) override;

private:
    /**
     * @brief Stores the evaluation context.
     */
    Json m_context;
};
}} // namespace jmespath::interpreter
#endif // EXPRESSIONEVALUATOR_H

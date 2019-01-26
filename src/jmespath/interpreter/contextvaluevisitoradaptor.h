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
#ifndef CONTEXTVALUEVISITORADAPTOR_H
#define CONTEXTVALUEVISITORADAPTOR_H
#include "jmespath/interpreter/interpreter2.h"
#include "jmespath/exceptions.h"
#include <boost/variant.hpp>

namespace jmespath{ namespace interpreter {

/**
 * @brief The ContextValueVisitorAdaptor class adapts an AbstractVisitor
 * implementation to the boost::static_visitor interface, so it can be used
 * to visit ast::interpreter::ContextValue objects.
 */
template <typename NodeT, typename VisitorT = Interpreter2>
class ContextValueVisitorAdaptor : public boost::static_visitor<>
{
public:
    /**
     * @brief Constructs a VariantVisitorAdaptor object with the given
     * \a visitor \a and node.
     * @param visitor The visitor object to which the visit calls will be
     * forwarded.
     * @param node Node that should be visited by the \a visitor.
     */
    ContextValueVisitorAdaptor(VisitorT* visitor, const NodeT *node)
        : boost::static_visitor<>{},
          m_visitor{visitor},
          m_node{node}
    {
        if (!m_visitor || !node)
        {
            BOOST_THROW_EXCEPTION(InvalidAgrument());
        }
    }
    /**
     * @brief Calls the appropriate visit method of the visitor object with the
     * address of the \a node and with the rvalue reference of \a value.
     * @param value The second parameter to the visit method of the \a visitor.
     */
    void operator() (Json& value) const
    {
        m_visitor->visit(m_node, std::move(value));
    }
    /**
     * @brief Calls the appropriate visit method of the visitor object with the
     * address of the \a node and with the const reference of the Json value
     * held by the \a value.
     * @param value The second parameter to the visit method of the \a visitor.
     */
    void operator() (const std::reference_wrapper<const Json>& value) const
    {
        m_visitor->visit(m_node, value.get());
    }

private:
    /**
     * @brief The visitor object to which the visit calls will be forwarded.
     */
    VisitorT* m_visitor;
    /**
     * @brief The node pointer which should be passed to the visit method of
     * the \a visitor.
     */
    const NodeT *m_node;
};
}} // namespace jmespath::interpreter
#endif // CONTEXTVALUEVISITORADAPTOR_H

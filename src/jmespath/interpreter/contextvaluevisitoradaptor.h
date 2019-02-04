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
#include <boost/hana.hpp>

namespace jmespath{ namespace interpreter {

/**
 * @brief The ContextValueVisitorAdaptor class adapts a visitor object,
 * which is callable with const lvalue reference of Json and with rvalue
 * reference of Json objects, to ast::interpreter::ContextValue objects.
 */
template <typename VisitorT, bool ForceMove = false>
class ContextValueVisitorAdaptor : public boost::static_visitor<>
{
public:
    /**
     * @brief Constructs a ContextValueVisitorAdaptor object, adapting the
     * \a visitor object to be able to consume ContextValue objects.
     * @param visitor A visitor which is callable with const lvalue reference
     * of Json and with rvalue reference of Json objects.
     */
    ContextValueVisitorAdaptor(VisitorT&& visitor)
        : boost::static_visitor<>{},
          m_visitor{std::move(visitor)}
    {
    }

    /**
     * @brief Calls the visitor object with the rvalue reference of the copy
     * of the object to which \a value refers to.
     * @param value A JsonRef value.
     */
    template <typename T>
    std::enable_if_t<std::is_same<T, JsonRef>::value && ForceMove, void>
    operator()(const T& value)
    {
        m_visitor(Json(value.get()));
    }

    /**
     * @brief Calls the visitor object with the lvalue reference of the Json
     * value to which \a value refers to.
     * @param value A JsonRef value.
     */
    template <typename T>
    std::enable_if_t<std::is_same<T, JsonRef>::value && !ForceMove, void>
    operator()(const T& value)
    {
         m_visitor(value.get());
    }

    /**
     * @brief Calls the visitor object with the rvalue reference of \a value.
     * @param value A Json value.
     */
    void operator()(Json& value)
    {
        m_visitor(std::move(value));
    }

private:
    /**
     * @brief The visitor object to which the calls will be forwarded.
     */
    VisitorT m_visitor;
};

/**
 * @brief Create visitor object which accepts ast::interpreter::ContextValue
 * objects, and calls @a lvalueFunc callable with a const lvalue ref of the Json
 * reference held by the ContextValue or calls the @a rvalueFunc callable with
 * an rvalue ref of the Json object held by ContextValue.
 * @param lvalueFunc A callable taking a const lvalue reference to Json.
 * @param rvalueFunc A callable taking an rvalue reference to Json.
 * @return A visitor object which accepts ast::interpreter::ContextValue objects
 */
inline decltype(auto) makeVisitor(
    std::function<void(const Json&)> &&lvalueFunc,
    std::function<void(Json&&)> &&rvalueFunc)
{
    auto functions = boost::hana::overload_linearly(
        std::move(rvalueFunc),
        std::move(lvalueFunc)
    );
    return ContextValueVisitorAdaptor<decltype(functions)>{
        std::move(functions)
    };
}

/**
 * @brief Create visitor object which accepts ast::interpreter::ContextValue
 * objects, and calls the @a rvalueFunc callable with an rvalue ref of the Json
 * object held by the ContextValue or calls it with the rvalue ref of the copy
 * of the object to which the Json reference points in the ContextValue object.
 * @param rvalueFunc A callable taking an rvalue reference to Json.
 * @return A visitor object which accepts ast::interpreter::ContextValue objects
 */
inline decltype(auto) makeMoveOnlyVisitor(
        std::function<void(Json&&)> rvalueFunc)
{
    auto result = ContextValueVisitorAdaptor<decltype(rvalueFunc), true>{
        std::move(rvalueFunc)
    };
    return result;
}
}} // namespace jmespath::interpreter
#endif // CONTEXTVALUEVISITORADAPTOR_H

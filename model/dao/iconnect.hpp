/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _MODEL_DAO_ICONNECT_HPP_
#define _MODEL_DAO_ICONNECT_HPP_

#include <string>
#include <memory>

namespace Model
{

namespace DAO
{

class IConnect
{

public:

    virtual ~IConnect() {}

    virtual uint_fast8_t init() = 0;

    virtual uint_fast8_t startConnect(const std::string &target,
                                 const int_fast32_t port = 0) = 0;

    std::shared_ptr<void> connectToken() const
    {
        return m_connectToken;
    }

    std::string targetPath() const
    {
        return m_targetPath;
    }

protected:

    std::shared_ptr<void> m_connectToken;

    std::string m_targetPath;

}; // end class IConnect

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_ICONNECT_HPP_

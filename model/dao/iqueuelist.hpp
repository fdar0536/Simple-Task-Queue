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

#ifndef _MODEL_DAO_IQUEUELIST_HPP_
#define _MODEL_DAO_IQUEUELIST_HPP_

#include <string>
#include <vector>

#include "iconnect.hpp"
#include "iqueue.hpp"

namespace Model
{

namespace DAO
{

class IQueueList
{
public:

    virtual ~IQueueList() {}

    virtual uint_fast8_t init(std::shared_ptr<IConnect> &connect) = 0;

    virtual uint_fast8_t createQueue(const std::string &name) = 0;

    virtual uint_fast8_t listQueue(std::vector<std::string> &out) = 0;

    virtual uint_fast8_t deleteQueue(const std::string &name) = 0;

    virtual uint_fast8_t renameQueue(const std::string &oldName,
                                const std::string &newName) = 0;

    virtual std::shared_ptr<IQueue> getQueue(const std::string &name) = 0;

protected:

    std::shared_ptr<IConnect> m_conn;

}; // end class IQueueList

} // end namespace DAO

} // end namespace Model

#endif // _MODEL_DAO_IQUEUELIST_HPP_

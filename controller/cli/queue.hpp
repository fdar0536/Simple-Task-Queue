/*
 * Simple Task Queue
 * Copyright (c) 2024 fdar0536
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

#ifndef _CONTROLLER_CLI_QUEUE_HPP_
#define _CONTROLLER_CLI_QUEUE_HPP_

#include <functional>
#include <memory>
#include <unordered_map>

#include "controller/global/defines.hpp"
#include "model/dao/iqueue.hpp"

namespace Controller
{

namespace CLI
{

class Queue
{
public:

    void init();

    i32 run(const std::string &, std::shared_ptr<Model::DAO::IQueue> &);

private:

    std::shared_ptr<Model::DAO::IQueue> m_queue;

    std::unordered_map<std::string, std::function<i32(void)>> m_funcs;

    i32 list();

    i32 details();

    i32 clear();

    i32 current();

    i32 add();

    i32 remove();

    i32 isRunning();

    i32 start();

    i32 stop();

    i32 output();

}; // end class Queue

} // end namesapce CLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_QUEUE_HPP_

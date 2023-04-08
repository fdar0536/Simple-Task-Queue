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

#include "errmsg.hpp"

namespace Model
{

ErrMsg::ErrMsg() :
    m_code(OK),
    m_msg("")
{}

void ErrMsg::setMsg(ErrCode code, const std::string &msg)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_code = code;
    m_msg = msg;
}

void ErrMsg::msg(ErrCode &code, std::string &msg)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    code = m_code;
    m_code = OK;

    msg = m_msg;
    m_msg.clear();
}

#ifndef STQ_MOBILE
grpc::Status ErrMsg::toGRPCStatus(ErrCode code, const std::string &msg)
{
    return grpc::Status(m_table[code], msg);
}
#endif

} // end namespace Model

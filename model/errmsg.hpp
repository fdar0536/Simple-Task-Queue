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

#ifndef _MODEL_IMODEL_HPP_
#define _MODEL_IMODEL_HPP_

#include <unordered_map>
#include <string>
#include <mutex>

#include "config.h"

#ifndef STQ_MOBILE
#include "grpcpp/grpcpp.h"
#endif

namespace Model
{

class ErrMsg
{
public:

    typedef enum ErrCode
    {
        OK = 0,
        INVALID_ARGUMENT,
        NOT_FOUND,
        ALREADY_EXISTS,
        OUT_OF_RANGE,
        OS_ERROR
    } ErrCode;

    ErrMsg();

    static void init();

    void setMsg(ErrCode, const std::string &);

    void msg(ErrCode * = nullptr, std::string * = nullptr);

#ifndef STQ_MOBILE
    static grpc::Status toGRPCStatus(ErrCode, const std::string &);
#endif

private:

    ErrCode m_code;

    std::string m_msg;

    std::mutex m_mutex;

}; // end class ErrMsg

} // end namespace Model

#endif // _MODEL_IMODEL_HPP_

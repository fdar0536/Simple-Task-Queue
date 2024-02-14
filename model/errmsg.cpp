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

namespace ErrMsg
{

#ifndef STQ_MOBILE
static std::unordered_map<uint_fast8_t, grpc::StatusCode> table;
#endif

void init()
{
#ifndef STQ_MOBILE
    table[ErrCode_OK] = grpc::StatusCode::OK;
    table[ErrCode_INVALID_ARGUMENT] = grpc::StatusCode::INVALID_ARGUMENT;
    table[ErrCode_NOT_FOUND] = grpc::StatusCode::NOT_FOUND;
    table[ErrCode_ALREADY_EXISTS] = grpc::StatusCode::ALREADY_EXISTS;
    table[ErrCode_OUT_OF_RANGE] = grpc::StatusCode::OUT_OF_RANGE;
    table[ErrCode_OS_ERROR] = grpc::StatusCode::INTERNAL;
#endif
}

#ifndef STQ_MOBILE
grpc::Status toGRPCStatus(uint_fast8_t code, const std::string &msg)
{
    return grpc::Status(table[code], msg);
}
#endif

} // end namespace ErrMsg

} // end namespace Model

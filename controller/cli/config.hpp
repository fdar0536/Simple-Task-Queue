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

#ifndef _CONTROLLER_CLI_CONFIG_HPP_
#define _CONTROLLER_CLI_CONFIG_HPP_

#include <string>

#include "spdlog/common.h"

#include "controller/global/defines.hpp"

#define BACKEND_GRPC   0
#define BACKEND_SQLITE 1

namespace Controller
{

namespace CLI
{

class Config
{
public:

    u8 backend = BACKEND_GRPC;

    std::string logFile = "";

    std::string address = "127.0.0.1";

    u16 port = 12345;

    bool autoConnect = true;

    i32 logLevel = static_cast<i32>(spdlog::level::level_enum::info);

}; // end class Config

} // end namesapce CLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_MAIN_HPP_

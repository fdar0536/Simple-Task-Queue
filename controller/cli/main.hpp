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

#ifndef _CONTROLLER_CLI_MAIN_HPP_
#define _CONTROLLER_CLI_MAIN_HPP_

#include "cxxopts.hpp"

#include "controller/global/defines.hpp"

namespace Controller
{

namespace CLI
{

class Main
{
public:

    Main();

    ~Main();

    i32 init(int, char **);

    i32 run();

private:

    i32 parseArgs(int, char **);

    void printVersion();

    u8 optsInit();

    i32 print();

    i32 modify();

    i32 connect();

    cxxopts::Options printOpts = cxxopts::Options("print", "print info");

    cxxopts::Options modifyOpts = cxxopts::Options("modify", "modify connection info");

}; // end class Main

} // end namesapce CLI

} // end namespace Controller

#endif // _CONTROLLER_CLI_MAIN_HPP_

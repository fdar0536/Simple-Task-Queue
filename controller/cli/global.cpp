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

#include <iostream>

#include "model/utils.hpp"

#include "global.hpp"

namespace Controller
{

namespace CLI
{

namespace Global
{

std::atomic<bool> keepRunning;

Controller::CLI::Config config;

std::vector<std::string> args;

void getArgs(const std::string &prefix)
{
    while (keepRunning.load(std::memory_order_relaxed))
    {
        args.clear();
        Model::Utils::writeConsole(prefix);
        std::string line;
        std::getline(std::cin, line);
        if (line.empty())
        {
            continue;
        }

        std::string delimiter = " ";

        size_t pos = 0;
        std::string token;
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            token = line.substr(0, pos);
            if (!token.empty())
            {
                args.push_back(token);
            }

            line.erase(0, pos + delimiter.length());
        }

        if (args.empty())
        {
            continue;
        }

        break;
    }
}

void printCMDHelp(const std::string &cmd)
{
    Model::Utils::writeConsole("Invalid argument\n");
    Model::Utils::writeConsole("Please type \"" + cmd + " help\" for more info\n");
}

} // end namespace Global

} // end namesapce CLI

} // end namespace Controller

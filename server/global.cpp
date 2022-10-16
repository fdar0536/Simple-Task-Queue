/*
 * Simple Task Queue
 * Copyright (c) 2022 fdar0536
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
#include <mutex>
#include <regex>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include "global.hpp"

namespace json = rapidjson;

namespace Global
{

Logger logger;

std::string fileSavePath;

std::string outFilePath;

std::string ip;

std::string port;

#ifdef BUILD_RESTFUL_SERVER

int restfulPort;

std::string webContextPath;

#endif

STQQueueList queueList;

std::promise<void> exit_requested;

static uint8_t verifyPath(std::string &path)
{
#ifdef _WIN32
    path += "\\";
#else
    path += "/";
#endif

    std::string filePath = path + "testFile";
    FILE *f;
    f = fopen(filePath.c_str(), "w+");
    if (!f)
    {
        std::cerr << filePath << " CANNOT open." << std::endl;
        return 1;
    }

    fputs(" \n", f);

    fclose(f);
    if (remove(filePath.c_str()))
    {
        std::cerr << filePath << " CANNOT remove." << std::endl;
        return 1;
    }

    return 0;
}

static uint8_t verifyPort(std::string &input, int *dst = nullptr)
{
    int tmpInt;
    if (sscanf(input.c_str(), "%d", &tmpInt) != 1)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid port." << std::endl;
        return 1;
    }

    if (tmpInt < 0 || tmpInt > 65535)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid port." << std::endl;
        return 1;
    }

    if (dst) *dst = tmpInt;
    return 0;
}

static uint8_t getJSONString(std::string &dst,
                             json::Document &doc,
                             const char *key)
{
    if (!key)
    {
        std::cout << __FILE__ << ":" << __LINE__;
        std::cout << " key is null." << std::endl;
        return 1;
    }

    if (!doc[key].IsString())
    {
        std::cout << key << " is not string" << std::endl;
        return 1;
    }

    dst = doc[key].GetString();
    return 0;
}

uint8_t init(const char *configFile)
{
    if (!configFile)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " \"configFile\" or \"res\" is null." << std::endl;
        return 1;
    }

    std::ifstream i(configFile);
    json::IStreamWrapper isw(i);
    json::Document j;
    j.ParseStream(isw);
    i.close();

    fileSavePath = j["file save path"].GetString();

    if (getJSONString(fileSavePath, j, "file save path"))
    {
        return 1;
    }

    if (verifyPath(fileSavePath))
    {
        return 1;
    }

    if (getJSONString(outFilePath, j, "output file path"))
    {
        return 1;
    }

    if (verifyPath(outFilePath))
    {
        return 1;
    }

    std::string tmpString;
    if (getJSONString(tmpString, j, "log path"))
    {
        return 1;
    }

    if (verifyPath(tmpString))
    {
        return 1;
    }

    logger.setSavePath(tmpString);

    if (getJSONString(ip, j, "ip"))
    {
        return 1;
    }

    std::regex ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                       "{3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    if (!std::regex_match(ip, ipRegex))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid ip address." << std::endl;
        return 1;
    }

    if (getJSONString(port, j, "port"))
    {
        return 1;
    }

    if (verifyPort(port))
    {
        return 1;
    }

#ifdef BUILD_RESTFUL_SERVER

    if (getJSONString(tmpString, j, "restful server port"))
    {
        return 1;
    }

    if (verifyPort(tmpString, &restfulPort))
    {
        return 1;
    }

    if (getJSONString(webContextPath, j, "web context path"))
    {
        return 1;
    }

#endif

    return 0;
}

} // end namespace Global

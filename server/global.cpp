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

#include "nlohmann/json.hpp"
#include "cppcodec/base32_rfc4648.hpp"

#include "global.hpp"
#include "model/stringutils.hpp"

using json = nlohmann::json;

namespace Global
{

Logger logger;

std::string fileSavePath;

std::string outFilePath;

std::string ip;

int port;

template<class T>
static uint8_t getConfigItem(T &dst, json &config, const char *entry)
{
    try
    {
        dst = config[entry];
    }
    catch (nlohmann::detail::type_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

static uint8_t verifyPath(std::string &path)
{
#ifdef _WIN32
    path += "\\";
#else
    path += "/";
#endif

    std::string filePath = path + "testFile";
    FILE *f;
    f = fopen(filePath.c_str(), "rw");
    if (!f)
    {
        std::cerr << filePath << " CANNOT open.";
        return 1;
    }

    fputs(" \n", f);

    fclose(f);
    if (remove(filePath.c_str()))
    {
        std::cerr << filePath << " CANNOT remove.";
        return 1;
    }

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
    json j = json::parse(i, nullptr, false);
    if (j.is_discarded())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to parse json." << std::endl;
        return 1;
    }

    i.close();

    if (getConfigItem(fileSavePath, j, "file save path"))
    {
        return 1;
    }

    if (verifyPath(fileSavePath))
    {
        return 1;
    }

    if (getConfigItem(outFilePath, j, "output file path"))
    {
        return 1;
    }

    if (verifyPath(outFilePath))
    {
        return 1;
    }

    if (getConfigItem(outFilePath, j, "output file path"))
    {
        return 1;
    }

    if (verifyPath(outFilePath))
    {
        return 1;
    }

    std::string tmpString;
    if (getConfigItem(tmpString, j, "log path"))
    {
        return 1;
    }

    if (verifyPath(tmpString))
    {
        return 1;
    }

    logger.setSavePath(tmpString);

    if (getConfigItem(ip, j, "ip"))
    {
        return 1;
    }

    if (getConfigItem(port, j, "port"))
    {
        return 1;
    }

    return 0;
}

} // end namespace Global

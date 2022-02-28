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

#include "stringutils.hpp"

namespace StringUtils
{

static uint8_t getCompressData(unsigned char in, unsigned char *out)
{
    if (!out) return 1;

    if (in <= '9')
    {
        *out = in - '0';
        return 0;
    }
    else if (in >= 'a' && in <= 'f')
    {
        *out = in - 87; // -'a' + 10 = -87
        return 0;
    }
    else if (in >= 'A' && in <= 'F')
    {
        *out = in - 55; // -'A' + 10 = -55
        return 0;
    }

    std::cerr << __FILE__ << ":" << __LINE__;
    std::cerr << " in is: " << in << std::endl;
    return 1;
}

static unsigned char decompressData[] = {'A', 'B', 'C', 'D', 'E', 'F'};

static uint8_t getDecompressData(unsigned char in, unsigned char *out)
{
    if (!out) return 1;

    if (in < 10)
    {
        *out = in + '0';
        return 0;
    }
    else if (in >= 10 && in <= 16)
    {
        // 10 ~ 16
        *out = decompressData[in - 10];
        return 0;
    }

    std::cerr << __FILE__ << ":" << __LINE__;
    std::cerr << " in is: " << in << std::endl;
    return 1;
}

uint8_t compress(const unsigned char *data, size_t dataLen,
                 unsigned char *output, size_t outputLen)
{
    if (!data || !dataLen || !output || !outputLen)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid input."<< std::endl;
        return 1;
    }

    if (dataLen & 0x1)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " dataLen is: " << dataLen << std::endl;
        return 1;
    }

    if (outputLen < (dataLen >> 1))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " dataLen is: " << dataLen;
        std::cerr << ", outputLen is: " << outputLen << std::endl;
        return 1;
    }

    unsigned char char1, char2;
    for (size_t i = 0; i < dataLen; i += 2)
    {
        if (getCompressData(data[i], &char1))
        {
            return 1;
        }

        if (getCompressData(data[i + 1], &char2))
        {
            return 1;
        }

        output[i >> 1] = ((char1 << 4) | char2);
    }

    return 0;
}

uint8_t decompress(const unsigned char *data, size_t dataLen,
                   unsigned char *output, size_t outputLen)
{
    if (!data || !dataLen || !output || !outputLen)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid input."<< std::endl;
        return 1;
    }

    if (outputLen < (dataLen << 1))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " dataLen is: " << dataLen;
        std::cerr << ", outputLen is: " << outputLen << std::endl;
        return 1;
    }

    unsigned char tmpChar;
    size_t index;
    for (size_t i = 0; i < dataLen; ++i)
    {
        index = (i << 1);
        tmpChar = ((data[i] & 0xf0) >> 4);
        if (getDecompressData(tmpChar, &output[index]))
        {
            return 1;
        }

        ++index;

        tmpChar = (data[i] & 0xf);
        if (getDecompressData(tmpChar, &output[index]))
        {
            return 1;
        }
    }

    return 0;
}

} // end namespace StringUtils

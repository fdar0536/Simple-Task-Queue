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

#include "totp.hpp"

namespace TOTP
{

static uint8_t getBase32DecodeData(uint8_t in, uint8_t *out)
{
    if (!out) return 1;

    if (in >= 'a' && in <= 'z')
    {
        *out = (in - 'a');
        return 0;
    }
    else if (in >= 'A' && in <= 'Z')
    {
        *out = (in - 'A');
        return 0;
    }
    else if (in >= '2' && in <= '7')
    {
        *out = (in - 24); // - '2' + 26 = -24
        return 0;
    }

    std::cerr << __FILE__ << ":" << __LINE__;
    std::cerr << " in is: " << in << std::endl;
    return 1;
}

uint8_t decodeBase32(const uint8_t *input,
                     size_t inputSize,
                     std::vector<uint32_t> *output)
{
    if (!input || !inputSize || !output)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid input."<< std::endl;
        return 1;
    }

    if (inputSize & 0x7)
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid inputSize."<< std::endl;
        return 1;
    }

    const uint8_t *ptr = input;
    uint64_t buf(0);
    uint8_t len(0);
    uint8_t tmpOut;

    for (size_t i = 0; i < inputSize; i += 8, ptr += 8)
    {
        len = 8;
        buf = 0;
        for (int j = 7; j >= 0; --j)
        {
            if (ptr[j] != '=')
            {
                break;
            }

            --len;
        }

        for (int j = 0; j < len; ++j)
        {
            if (getBase32DecodeData(ptr[j], &tmpOut))
            {
                return 1;
            }

            buf <<= 5;
            buf |= tmpOut;
        }


    }

    return 0;
}

uint8_t getTOTP(uint8_t *);

} // end namespace TOTP

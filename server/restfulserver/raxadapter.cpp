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

#include "cstdlib"

extern "C"
{

#include "rax/rax.h"

}

#include "raxadapter.hpp"

RaxAdapter::RaxAdapter() :
    m_rax(nullptr)
{}

RaxAdapter::~RaxAdapter()
{
    if (m_rax)
    {
        raxFreeWithCallback(m_rax, free);
    }
}

uint8_t RaxAdapter::init(RaxAdapter &in)
{
    in.m_rax = raxNew();
    if (!in.m_rax) return 1;

    return 0;
}

int RaxAdapter::insert(uint8_t *key, size_t keyLen, HttpUtils::Handler *in)
{
    if (!in) return 0;
    return raxInsert(m_rax, key, keyLen, in, NULL);
}

HttpUtils::Handler *RaxAdapter::find(uint8_t *key, size_t keyLen)
{
    void *data(raxFind(m_rax, key, keyLen));
    if (data == raxNotFound)
    {
        return nullptr;
    }

    return reinterpret_cast<HttpUtils::Handler *>(data);
}

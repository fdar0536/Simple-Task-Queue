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

#pragma once

#include <inttypes.h>

#define HTTP_CONTINUE    1
#define HTTP_END         0
#define HTTP_BUFFER_SIZE 4194304 // 4 * 1024 * 1024

namespace HttpUtils
{

typedef struct Buffer
{
    uint8_t data[HTTP_BUFFER_SIZE] = {};

    uint8_t *patchPtr = nullptr;

    size_t patchedSize = 0;
} Buffer;

typedef uint8_t (*HandlerFunc)(Buffer *in,
                               Buffer *out,
                               void **status);

typedef struct Handler
{
    HandlerFunc Get = nullptr;

    HandlerFunc Post = nullptr;

    HandlerFunc Put = nullptr;

    HandlerFunc Delete = nullptr;
} Handler;

uint8_t init();

uint8_t resetBuffer(const Buffer *in);

uint8_t patchBuffer(const uint8_t *data,
                    const size_t dataLen,
                    Buffer *out);

uint8_t patchHead(const char *code,
                  const size_t codeLen,
                  Buffer *out);

uint8_t patchField(const char *name,
                   const size_t nameLen,
                   const char *data,
                   const size_t dataLen,
                   Buffer *out);

uint8_t patchContentType(const char *name,
                         const size_t nameLen,
                         Buffer *out);

uint8_t patchEnd(const Buffer *out);

uint8_t patchDataAndEnd(const uint8_t *data,
                        const size_t dataLen,
                        Buffer *out);

uint8_t patchError(const char *code,
                   const uint8_t codeLen,
                   const uint8_t *reason,
                   const size_t reasonLen,
                   Buffer *out);

}; // end namespace HttpUtils

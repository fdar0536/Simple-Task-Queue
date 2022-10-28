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

#include <unordered_map>

#include "httputils.hpp"

namespace HttpUtils
{

static std::unordered_map<std::string, std::string> responseMsg;

uint8_t init()
{
    responseMsg.reserve(4);
    try
    {
        // 2xx
        responseMsg["200"] = " OK\r\n";

        // 4xx
        responseMsg["400"] = " Bad Request\r\n";
        responseMsg["403"] = " Forbidden\r\n";
        responseMsg["404"] = " Not Found\r\n";

        // 5xx
        responseMsg["500"] = " Internal Server Error\r\n";
    }
    catch(...)
    {
        return 1;
    }

    return 0;
}

uint8_t resetBuffer(Buffer *in)
{
    if (!in) return 1;

    in->patchPtr = nullptr;
    in->patchedSize = 0;
    return 0;
}

uint8_t patchBuffer(const uint8_t *data,
                    const size_t dataLen,
                    Buffer *out)
{
    if (!data || !dataLen || !out) return 1;

    if (out->patchedSize == HTTP_BUFFER_SIZE) return 1;

    size_t newSize = out->patchedSize + dataLen;
    if (newSize >= HTTP_BUFFER_SIZE) return 1;

    memcpy(out->patchPtr, data, dataLen);
    out->patchPtr += dataLen;
    out->patchedSize = newSize;

    return 0;
}

uint8_t patchHead(const char *code,
                  const size_t codeLen,
                  Buffer *out)
{
    // "patchBuffer" will check the input
    // so no need to check them here

    std::string data = responseMsg[code];
    if (data.empty()) return 1;

    if (patchBuffer(reinterpret_cast<const unsigned char *>(code),
                    codeLen,
                    out))
    {
        return 1;
    }

    if (patchBuffer(reinterpret_cast<const unsigned char *>(data.c_str()),
                    data.size(),
                    out))
    {
        return 1;
    }

    return 0;
}

uint8_t patchField(const char *name,
                   const size_t nameLen,
                   const char *data,
                   const size_t dataLen,
                   Buffer *out)
{
    // "patchBuffer" will check the input
    // so no need to check them here

    if (patchBuffer(reinterpret_cast<const unsigned char *>(name),
                    nameLen,
                    out))
    {
        return 1;
    }

    if (patchBuffer(reinterpret_cast<const unsigned char *>(": "),
                    2,
                    out))
    {
        return 1;
    }

    if (patchBuffer(reinterpret_cast<const unsigned char *>(data),
                    dataLen,
                    out))
    {
        return 1;
    }

    if (patchEnd(out))
    {
        return 1;
    }

    return 0;
}

uint8_t patchContentType(const char *name,
                         const size_t nameLen,
                         Buffer *out)
{
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type
    if (patchField("Content-Type", 12, name, nameLen, out))
    {
        return 1;
    }

    if (patchField("X-Content-Type-Options", 22, "nosniff", 7, out))
    {
        return 1;
    }

    return 0;
}

uint8_t patchEnd(Buffer *out)
{
    // "patchBuffer" will check the input
    // so no need to check them here

    if (patchBuffer(reinterpret_cast<const unsigned char *>("\r\n"),
                    2,
                    out))
    {
        return 1;
    }

    return 0;
}

uint8_t patchDataAndEnd(const uint8_t *data,
                        const size_t dataLen,
                        Buffer *out)
{
    // "patchBuffer" will check the input
    // so no need to check them here

    if (patchEnd(out))
    {
        return 1;
    }

    if (patchBuffer(reinterpret_cast<const unsigned char *>(data),
                    dataLen,
                    out))
    {
        return 1;
    }

    if (patchEnd(out))
    {
        return 1;
    }

    return 0;
}

uint8_t patchError(const char *code,
                   const uint8_t codeLen,
                   const uint8_t *reason,
                   const size_t reasonLen,
                   Buffer *out)
{
    // "patchBuffer" will check the input
    // so no need to check them here

    uint8_t *buf = new (std::nothrow) uint8_t[reasonLen + 13]();
    if (!buf) return 1;

    uint8_t ret(0);
    uint8_t *ptr = buf;

    if (patchHead(code, codeLen, out))
    {
        ret = 1;
        goto error;
    }

    if (patchContentType("application/json", 16, out))
    {
        ret = 1;
        goto error;
    }

    memcpy(ptr, "{\"reason\":\"", 11);
    ptr += 11;

    memcpy(ptr, reason, reasonLen);
    ptr += reasonLen;

    memcpy(ptr, "\"}", 2);
    ptr += 2;

    if (patchDataAndEnd(buf, ptr - buf, out))
    {
        ret = 1;
    }

error:

    delete[] buf;
    return ret;
}

}; // end namespace HttpUtils

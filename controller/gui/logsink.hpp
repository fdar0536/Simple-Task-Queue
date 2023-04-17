/* This file is modified from spdlog
 * Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#ifndef _CONTROLLER_GUI_LOGSINK_HPP_
#define _CONTROLLER_GUI_LOGSINK_HPP_

#include "spdlog/sinks/base_sink.h"

#include "main.hpp"

namespace Controller
{

namespace GUI
{

template<typename Mutex>
class LogSink : public spdlog::sinks::base_sink<Mutex>
{

public:

    LogSink(Main *in)
    {
        m_main = in;
    }

    ~LogSink()
    {
        flush_();
    }

protected:

    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        spdlog::string_view_t str = spdlog::string_view_t(formatted.data(),
                                                          formatted.size());
        m_main->onSpdlogLog(QString::fromUtf8(
                                str.data(),
                                static_cast<int>(str.size())).trimmed());
    }

    void flush_() override {}

private:

    Main *m_main;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using LogSink_mt = LogSink<std::mutex>;
using LogSink_st = LogSink<spdlog::details::null_mutex>;

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_LOGSINK_HPP_

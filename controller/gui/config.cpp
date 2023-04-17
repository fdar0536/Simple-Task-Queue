/* Simple Task Queue
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

#include "controller/global/init.hpp"
#include "config.hpp"

namespace Controller
{

namespace GUI
{

Config::Config(QObject *parent) :
    QObject(parent)
{
    m_isInit.store(false, std::memory_order_relaxed);
}

Config::~Config()
{}

bool Config::init()
{
    return true;
}

bool Config::isLocalAvailable() const
{
#ifdef STQ_MOBILE
    return false;
#else
    return Controller::Global::sqliteQueueList != nullptr;
#endif
}

bool Config::isServerRunning() const
{
#ifdef STQ_MOBILE
    return false;
#else
    return Controller::Global::server.isRunning();
#endif
}

} // namespace GUI

} // end namespace Controller

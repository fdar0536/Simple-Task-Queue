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

#ifndef _CONTROLLER_GUI_CONFIGCONFIGTHREAD_HPP_
#define _CONTROLLER_GUI_CONFIGCONFIGTHREAD_HPP_

#include "QThread"

namespace Controller
{

namespace GUI
{

class ClientConfigThread : public QThread
{
    Q_OBJECT

public:

    ClientConfigThread(QObject * = nullptr);

    ~ClientConfigThread();

    uint_fast8_t startParseConfig(QHash<QString, QVariant> *);

    uint_fast8_t startConnect(const QString &, const int_fast32_t);

    virtual void run() override;

signals:

    void parseConfigDone();

    void connectDone(bool);

private:

    std::string m_host;

    int_fast32_t m_port;

    std::atomic<bool> m_isRunning = false;

    QHash<QString, QVariant> *m_data = nullptr;

    typedef void (ClientConfigThread::*Handler)();

    typedef enum Mode
    {
        PARSECONFIG, CONNECT
    } Mode;

    Handler m_handler[2] =
    {
        &ClientConfigThread::parseConfigImpl,
        &ClientConfigThread::connectImpl
    };

    std::atomic<Mode> m_mode;

    void parseConfigImpl();

    void connectImpl();

}; // end class ClientConfigThread

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_CONFIGCONFIGTHREAD_HPP_

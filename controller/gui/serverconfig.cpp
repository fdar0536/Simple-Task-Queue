/*
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

#include "QCoreApplication"

#include "controller/global/init.hpp"
#include "controller/gui/global.hpp"
#include "serverconfig.hpp"

namespace Controller
{

namespace GUI
{

ServerConfig::ServerConfig(QObject *parent) :
    QThread(parent)
{
}

ServerConfig::~ServerConfig()
{}

void ServerConfig::init()
{
    start();
}

bool ServerConfig::isServerRunning() const
{
#ifdef STQ_MOBILE
    return false;
#else
    return Controller::Global::server.isRunning();
#endif
}

bool ServerConfig::autoStartServer() const
{
#ifdef STQ_MOBILE
    return false;
#else
    return Controller::Global::config.autoStartServer();
#endif
}

void ServerConfig::setAutoStartServer(bool in)
{
#ifdef STQ_MOBILE
    static_cast<void>(in);
#else
    Controller::Global::config.setAutoStartServer(in);
#endif
}

QString ServerConfig::serverIP() const
{
#ifdef STQ_MOBILE
    return "";
#else
    return QString::fromStdString(Controller::Global::config.listenIP());
#endif
}

bool ServerConfig::setServerIP(const QString &in)
{
#ifdef STQ_MOBILE
    static_cast<void>(in);
    return false;
#else
    if (Controller::Global::config.setListenIP(in.toUtf8().toStdString()))
    {
        return false;
    }

    return true;
#endif
}

int ServerConfig::serverPort() const
{
#ifdef STQ_MOBILE
    return 0;
#else
    return static_cast<int>(Controller::Global::config.listenPort());
#endif
}

void ServerConfig::setServerPort(const int in)
{
#ifdef STQ_MOBILE
    static_cast<void>(in);
#else
    Controller::Global::config.setListenPort(static_cast<uint_fast16_t>(in));
#endif
}

bool ServerConfig::startServer()
{
#ifdef STQ_MOBILE
    return false;
#else
    if (Controller::Global::server.start())
    {
        return false;
    }

    return true;
#endif
}

void ServerConfig::stopServer()
{
#ifndef STQ_MOBILE
    Controller::Global::server.stop();
#endif
}

void ServerConfig::run()
{
    if (!Controller::Global::guiGlobal.isNotMobile())
    {
        emit InitDone();
        return;
    }

    uint_fast8_t count(0);
    while (!Controller::Global::guiGlobal.isLocalAvailable())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        ++count;
        if (count > 5)
        {
            break;
        }
    }

    emit InitDone();
}

} // namespace GUI

} // end namespace Controller

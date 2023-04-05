/*
 * Simple Task Queue
 * Copyright (c) 2022-2023 fdar0536
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

#include <csignal>

#ifdef _WIN32
#include "windows.h"
#endif

#include "spdlog/spdlog.h"
#include "QQmlApplicationEngine"

#include "controller/global/init.hpp"

#ifdef STQ_GUI

#ifdef STQ_MOBILE
#include "QGuiApplication"
#else
#include "QApplication"
#endif // STQ_MOBILE

#include "QIcon"

#endif // STQ_GUI

static bool isAdmin();

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

int main(int argc, char **argv)
{
    if (isAdmin())
    {
#ifdef _WIN32
        spdlog::error("{}:{} Refuse to run as administrator", __FILE__, __LINE__);
#else
        spdlog::error("{}:{} Refuse to run as super user", __FILE__, __LINE__);
#endif
        return 1;
    }

    if (Controller::Global::init(argc, argv))
    {
        spdlog::error("{}:{} Fail to initialize", __FILE__, __LINE__);
        return 1;
    }

    signal(SIGABRT, sighandler);
    signal(SIGFPE,  sighandler);
    signal(SIGILL,  sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

#ifdef _WIN32
    SetConsoleCtrlHandler(eventHandler, TRUE);
#endif

    int ret(0);
#ifdef STQ_GUI

#ifdef STQ_MOBILE
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif // STQ_MOBILE

    app.setWindowIcon(QIcon(":/STQ.ico"));

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/stq.qml")));
    if (engine.rootObjects().isEmpty())
    {
        spdlog::error("{}:{} Fail to load qml", __FILE__, __LINE__);
        ret = 1;
        goto exit;
    }

#ifdef STQ_MOBILE
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
#else
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &QApplication::quit);
#endif // STQ_MOBILE

    ret = app.exec();
#else
    if (Controller::Global::server.start())
    {
        spdlog::error("{}:{} Fail to start server", __FILE__, __LINE__);
        ret = 1;
    }
#endif // STQ_GUI

exit:
    Controller::Global::fin();
    return ret;
}

static bool isAdmin()
{
#ifdef _WIN32
    PSID sid;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&auth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &sid))
    {
        return true;
    }

    BOOL res;
    if (!CheckTokenMembership(nullptr, sid, &res))
    {
        return true;
    }

    FreeSid(sid);
    return res;
#else
    return (geteuid() == 0);
#endif
}

static void sighandler(int signum)
{
    UNUSED(signum);
    spdlog::info("{}:{} Good Bye!", __FILE__, __LINE__);
#ifdef STQ_GUI

#else
    Controller::Global::server.stop();
#endif
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    UNUSED(dwCtrlType);
    sighandler(0);
    return TRUE;
}
#endif
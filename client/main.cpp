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

#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif

#include <new>

#include "QApplication"
#include "QMessageBox"
#include "QQmlApplicationEngine"
#include "QQuickStyle"

#include "model/global.hpp"
#include "model/mainmodel.hpp"
#include "model/settingsmodel.hpp"

static bool isAdmin();
static uint8_t init(QQmlApplicationEngine *engine);

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/ui/icon/computer_black_48dp.svg"));
    if (isAdmin())
    {
#ifdef _WIN32
        QMessageBox::critical(nullptr,
                              a.tr("Error"),
                              a.tr("Refuse to run as administrator."));
#else
        QMessageBox::critical(nullptr,
                              a.tr("Error"),
                              a.tr("Refuse to run as super user."));
#endif
        return 1;
    }

    QQmlApplicationEngine *engine = new (std::nothrow) QQmlApplicationEngine;
    if (!engine)
    {
        QMessageBox::critical(nullptr,
                              a.tr("Error"),
                              a.tr("Fail to create qml engine."));
        return 1;
    }

    if (init(engine))
    {
        QMessageBox::critical(nullptr,
                              a.tr("Error"),
                              a.tr("Fail to initialize."));
        return 1;
    }

    const int ret = a.exec();
    delete engine;
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

static uint8_t init(QQmlApplicationEngine *engine)
{
    QQuickStyle::setStyle("Material");
    Global *global(nullptr);
    try
    {
        global = Global::instance(engine);
        if (!global) return 1;
    }
    catch (...)
    {
        return 1;
    }

    qmlRegisterSingletonType<Global>("model.Global",
                                     0,
                                     1,
                                     "Global",
                                     [=](QQmlEngine *, QJSEngine *) -> QObject *
                                     {
                                         return global;
                                     });

    qmlRegisterType<MainModel>("model.MainModel", 0, 1, "MainModel");
    qmlRegisterType<SettingsModel>("model.SettingsModel", 0, 1, "SettingsModel");

    engine->load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
    if (engine->rootObjects().isEmpty())
        return 1;

    return 0;
}

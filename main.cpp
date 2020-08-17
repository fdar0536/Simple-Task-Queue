/*
 * Simple Task Queue
 * Copyright (c) 2020 fdar0536
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

#ifdef _WIN32
#include "windows.h"
#else
#include "unistd.h"
#endif

#include "QApplication"
#include "QMessageBox"
#include "QDir"
#include "QFileInfo"
#include "QFile"

#include "mainwindow.hpp"

bool isAdmin()
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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (isAdmin())
    {
#ifdef _WIN32
        QMessageBox::critical(nullptr,
                              QCoreApplication::tr("Fatal error"),
                              QCoreApplication::tr("Administrator CANNOT run this program!"));
#else
        QMessageBox::critical(nullptr,
                              QCoreApplication::tr("Fatal error"),
                              QCoreApplication::tr("Super user CANNOT run this program"));
#endif
        return 1;
    }

    QString lockFilePath(QDir::tempPath() + "/STQ.lock");
    QFile file(lockFilePath);
    if (file.exists())
    {
        QMessageBox::information(nullptr,
                                 QCoreApplication::tr("Hint"),
                                 QCoreApplication::tr("This program is already running."));
        return 0;
    }

    if (!file.open(QIODevice::Append))
    {
        QMessageBox::critical(nullptr,
                              QCoreApplication::tr("Fatal error"),
                              QCoreApplication::tr("Fail to open lock file."));
        return 1;
    }

    file.write("lock\n");
    file.flush();
    file.close();

    MainWindow *w(MainWindow::create());
    if (!w)
    {
        QMessageBox::critical(nullptr,
                              QCoreApplication::tr("Fatal error"),
                              QCoreApplication::tr("Fail to create main window!"));
        file.remove();
        return 1;
    }

    w->show();

    const int res = a.exec();
    delete w;
    file.remove();
    return res;
}

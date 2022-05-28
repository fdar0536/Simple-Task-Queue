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

#include "QApplication"
#include "QMessageBox"

#include "ui/mainwindow.hpp"

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

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
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

    MainWindow w;
    if (MainWindow::init(&w))
    {
        QMessageBox::critical(nullptr,
                              a.tr("Error"),
                              a.tr("Fail to initialize."));
        return 1;
    }

    w.show();
    return a.exec();
}

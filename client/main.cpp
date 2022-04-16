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

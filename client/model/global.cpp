#include "QApplication"
#include "QMessageBox"

#include "global.hpp"

// public member functions
Global::Global() :
    QObject()
{}

Global::~Global()
{}

void Global::exit()
{
    auto res(QMessageBox::question(nullptr,
                                   tr("Exit"),
                                   tr("Are you sure to exit?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));
    if (res == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

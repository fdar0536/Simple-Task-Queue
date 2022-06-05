#include <new>

#include "QApplication"
#include "QMessageBox"

#include "global.hpp"

// public member functions
Global::~Global()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon) delete m_icon;
        if (m_iconContextMenu) delete m_iconContextMenu;
        if (m_showAction) delete m_showAction;
        if (m_exitAction) delete m_exitAction;
    }
}

Global *Global::create()
{
    Global *ret(nullptr);

    ret = new (std::nothrow) Global;
    if (!ret) return nullptr;

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        ret->m_icon = new (std::nothrow) QSystemTrayIcon(QIcon(":/ui/icon/computer_black_48dp.svg"), ret);
        if (!ret->m_icon)
        {
            delete ret;
            return nullptr;
        }

        ret->m_icon->show();

        //iconContextMenu
        ret->m_iconContextMenu = new (std::nothrow) QMenu(ret);
        if (!ret->m_iconContextMenu)
        {
            delete ret;
            return nullptr;
        }

        ret->m_showAction = new (std::nothrow) QAction("Show", ret);
        if (!ret->m_showAction)
        {
            delete ret;
            return nullptr;
        }

        ret->m_exitAction = new (std::nothrow) QAction("Exit", ret);
        if (!ret->m_exitAction)
        {
            delete ret;
            return nullptr;
        }

        ret->m_iconContextMenu->addAction(ret->m_showAction);
        ret->m_iconContextMenu->addAction(ret->m_exitAction);
        ret->m_icon->setContextMenu(ret->m_iconContextMenu);
    }

    ret->connectHook();
    return ret;
}

void Global::setState(QString key, QVariantMap data)
{
    m_stateStore[key] = data;
}

QVariantMap Global::getState(QString key)
{
    auto it = m_stateStore.find(key);
    if (it == m_stateStore.end())
    {
        return QVariantMap();
    }

    return it.value();
}

// public slots
void Global::programExit()
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

// private slots
// Tray Icon
void Global::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        emit showWindow();
    }
}

// private member functions
Global::Global() :
    QWidget(),
    m_icon(nullptr),
    m_iconContextMenu(nullptr),
    m_showAction(nullptr),
    m_exitAction(nullptr)
{}

void Global::connectHook()
{
    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(m_icon,
                &QSystemTrayIcon::activated,
                this,
                &Global::iconActivated);

        connect(m_showAction,
                &QAction::triggered,
                this,
                &Global::showWindow);

        connect(m_exitAction,
                &QAction::triggered,
                this,
                &Global::programExit);
    }
}

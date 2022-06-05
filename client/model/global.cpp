#include <iostream>
#include <new>

#include "QApplication"
#include "QDir"
#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJSValue"
#include "QMessageBox"
#include "QRegularExpressionValidator"
#include "QStandardPaths"

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

Global *Global::create(QQmlApplicationEngine *engine)
{
    if (!engine) return nullptr;
    Global *ret(nullptr);

    ret = new (std::nothrow) Global;
    if (!ret) return nullptr;

    ret->m_engine = engine;
    if (ret->initConfigFile())
    {
        delete ret;
        return nullptr;
    }

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

void Global::setState(QString key, QJSValue data)
{
    m_stateStore[key] = data;
}

void Global::aboutQt()
{
    QMessageBox::aboutQt(this);
}

QJSValue Global::getState(QString key)
{
    auto it = m_stateStore.find(key);
    if (it == m_stateStore.end())
    {
        return m_engine->newObject();
    }

    return it.value();
}

void Global::saveSettings(QJSValue input)
{
    m_stateStore["settings"] = input;
    QJSValue list = input.property("config");
    qint32 length = list.property("length").toInt();

    if (length == 0) return;

    QString path = input.property("configPath").toString();
    qDebug() << "path is: " << path;
    QFile config(path);
    if (!config.open(QIODevice::WriteOnly))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to open config file." << std::endl;
        return;
    }

    QJsonObject json;
    QJsonArray arr;

    for (qsizetype i = 0; i < length; ++i)
    {
        QJsonObject item;
        QJSValue map = list.property(i);
        item.insert("alias", map.property("alias").toString());
        item.insert("ip", map.property("ip").toString());
        item.insert("port", map.property("port").toInt());
        arr.append(item);
    }

    json.insert("config", arr);
    QJsonDocument doc(json);
    config.write(doc.toJson());
    config.close();
}

QJSValue Global::getSettings()
{
    return m_stateStore["settings"];
}

void Global::setIsSettingsAccept(bool input)
{
    m_isSettingsAccept = input;
}

bool Global::isSettingsAccept() const
{
    return m_isSettingsAccept;
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
    m_exitAction(nullptr),
    m_isSettingsAccept(false),
    m_engine(nullptr)
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

uint8_t Global::initConfigFile()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configPath.isEmpty())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to determine config location." << std::endl;
        return 1;
    }

    QDir dir;
    if (!dir.mkpath(configPath))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to create config location." << std::endl;
        return 1;
    }

    configPath += "/config.json";
    QJSValue output = m_engine->newObject();
    output.setProperty("configPath", QJSValue(configPath));

    QFile config(configPath);
    if (!config.open(QIODevice::ReadOnly))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to open config file." << std::endl;
        return 0;
    }

    QByteArray data = config.readAll();
    config.close();
    QJsonObject configObj = QJsonDocument::fromJson(data).object();

    if (!configObj["config"].isArray())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Invalid config file." << std::endl;
        return 0;
    }

    QJsonArray arr = configObj["config"].toArray();
    QString tmpString;
    QJSValue list = m_engine->newArray(arr.size());

    QRegularExpression re("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                          "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    re.optimize();

    QRegularExpressionValidator regex = QRegularExpressionValidator(re, nullptr);
    int pos = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();
        QJSValue out = m_engine->newObject();

        if (!obj["alias"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("alias", QJSValue(obj["alias"].toString()));
        if (!obj["ip"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        tmpString = obj["ip"].toString();
        if (regex.validate(tmpString, pos) != QValidator::Acceptable)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid ip." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("ip", QJSValue(tmpString));

        if (!obj["port"].isDouble())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        pos = obj["port"].toInt();
        if (pos > 65535 || pos < 0)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid port." << std::endl;
            list = m_engine->newArray();
            goto exit;
        }

        out.setProperty("port", QJSValue(pos));
        list.setProperty(i, out);
    }

exit:

    output.setProperty("config", list);
    m_stateStore["settings"] = output;
    return 0;
}

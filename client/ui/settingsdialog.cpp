#include <new>
#include <iostream>

#include "QDir"
#include "QFile"
#include "QJsonDocument"
#include "QJsonArray"
#include "QStandardPaths"

#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"

SettingsDialog *SettingsDialog::create(QWidget *parent)
{
    SettingsDialog *ret(new (std::nothrow) SettingsDialog(parent));
    if (!ret) return nullptr;

    ret->m_ui = new (std::nothrow) Ui::SettingsDialog;
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->setFixedSize(675, 185);

    QRegularExpression re("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                          "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    re.optimize();

    ret->m_regex = new (std::nothrow) QRegularExpressionValidator(re, ret);
    if (!ret->m_regex)
    {
        delete ret;
        return nullptr;
    }

    if (ret->initConfigFile())
    {
        delete ret;
        return nullptr;
    }

    if (ret->m_config.size() > 0)
    {
        for (auto it = ret->m_config.begin();
            it != ret->m_config.end();
            ++it)
        {
            ret->m_ui->hosts->insertItem(ret->m_ui->hosts->count() + 1,
                it.key());
        }

        ret->m_ui->hosts->setCurrentIndex(0);
        ret->setupData(&ret->m_config.begin().value());
    }
    else
    {
        ret->m_ui->hosts->setCurrentIndex(-1);
    }

    return ret;
}

SettingsDialog::~SettingsDialog()
{
    saveConfigFile();
    if (m_ui) delete m_ui;
    if (m_regex) delete m_regex;
}

// private slots
void SettingsDialog::on_hosts_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    setupData(&m_config[m_ui->hosts->currentText()]);
}

// private member functions
SettingsDialog::SettingsDialog(QWidget *parent):
    QDialog(parent),
    m_ui(nullptr),
    m_regex(nullptr)
{}

uint8_t SettingsDialog::initConfigFile()
{
    m_config.clear();
    m_configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (m_configPath.isEmpty())
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to determine config location." << std::endl;
        return 1;
    }

    QDir dir;
    if (!dir.mkpath(m_configPath))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to create config location." << std::endl;
        return 1;
    }

    m_configPath += "/config.json";
    QFile config(m_configPath);
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
    int pos = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();

        if (!obj["alias"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        QString key = obj["alias"].toString();
        if (m_config.find(key) != m_config.end())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        if (!obj["ip"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        tmpString = obj["ip"].toString();
        if (m_regex->validate(tmpString, pos) != QValidator::Acceptable)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid ip." << std::endl;
            m_config.clear();
            return 0;
        }

        SettingsData data;
        data.ip = tmpString;

        if (!obj["port"].isDouble())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        pos = obj["port"].toInt();
        if (pos > 65535 || pos < 0)
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid port." << std::endl;
            m_config.clear();
            return 0;
        }

        data.port = static_cast<uint16_t>(pos);

        if (!obj["configFile"].isString())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        tmpString = obj["configFile"].toString();
        if (tmpString.isEmpty())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        data.configFile = tmpString;

        if (!obj["isLocalHost"].isDouble())
        {
            std::cerr << __FILE__ << ":" << __LINE__;
            std::cerr << " Invalid config file." << std::endl;
            m_config.clear();
            return 0;
        }

        data.isLocalHost = static_cast<uint16_t>(obj["isLocalHost"].toInt());
        m_config[key] = data;
    }

    return 0;
}

void SettingsDialog::saveConfigFile()
{
    if (!m_config.size()) return;

    QFile config(m_configPath);
    if (!config.open(QIODevice::WriteOnly))
    {
        std::cerr << __FILE__ << ":" << __LINE__;
        std::cerr << " Fail to open config file." << std::endl;
        return;
    }

    QJsonObject json;
    QJsonArray arr;
    for (auto it = m_config.begin();
        it != m_config.end();
        ++it)
    {
        QJsonObject item;
        item.insert("alias", it.key());
        item.insert("ip", it.value().ip);
        item.insert("port", it.value().port);
        item.insert("configFile", it.value().configFile);
        item.insert("isLocalHost", it.value().isLocalHost);
        arr.append(item);
    }

    json.insert("config", arr);
    QJsonDocument doc(json);
    config.write(doc.toJson());
    config.close();
}

void SettingsDialog::setupData(SettingsData *data)
{
    if (!data) return;

    m_ui->ip->setText(data->ip);
    m_ui->port->setValue(data->port);
    m_ui->configPath->setText(data->configFile);
    m_ui->localHost->setChecked(data->isLocalHost);
}

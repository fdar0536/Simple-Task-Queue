#include <new>
#include <iostream>

#include "QDir"
#include "QFile"
#include "QJsonDocument"
#include "QJsonArray"
#include "QStandardPaths"

#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"

// public member functions
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
    ret->setFixedSize(375, 25);

    QRegularExpression re("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.)"
                          "{3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    re.optimize();

    ret->m_regex = new (std::nothrow) QRegularExpressionValidator(re, ret);
    if (!ret->m_regex)
    {
        delete ret;
        return nullptr;
    }

    ret->m_saveConfigDialog = SaveConfigDialog::create(&ret->m_config, ret);
    if (!ret->m_saveConfigDialog)
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
        ret->m_ui->connectBtn->setEnabled(true);
        ret->m_ui->saveBtn->setEnabled(true);
        ret->m_ui->deleteBtn->setEnabled(true);
    }
    else
    {
        ret->m_ui->hosts->setCurrentIndex(-1);
    }

    ret->connect(
        ret->m_saveConfigDialog,
        &SaveConfigDialog::accepted,
        ret,
        &SettingsDialog::onSaveAccepted
    );

    return ret;
}

SettingsDialog::~SettingsDialog()
{
    saveConfigFile();
    if (m_ui) delete m_ui;
    if (m_regex) delete m_regex;
    if (m_saveConfigDialog) delete m_saveConfigDialog;
}

void SettingsDialog::reject()
{
    on_exitBtn_clicked(true);
}

// private slots
void SettingsDialog::on_hosts_currentIndexChanged(int)
{
    if (m_config.empty()) return;
    setupData(&m_config[m_ui->hosts->currentText()]);
}

void SettingsDialog::on_ip_textChanged(const QString &)
{
    verifyIP();
}

void SettingsDialog::on_port_valueChanged(int)
{
    verifyIP();
}

void SettingsDialog::on_saveBtn_clicked(bool)
{
    m_saveConfigDialog->open();
}

void SettingsDialog::on_exitBtn_clicked(bool)
{

}

void SettingsDialog::onSaveAccepted()
{
    QString key(m_saveConfigDialog->getName());
    m_saveConfigDialog->reset();

    SettingsData data;
    data.ip = m_ui->ip->text();
    data.port = static_cast<uint16_t>(m_ui->port->value());
    m_config[key] = data;
    
    m_ui->hosts->insertItem(m_ui->hosts->count() + 1, key);
    m_ui->hosts->setCurrentIndex(m_ui->hosts->count() - 1);
}

// private member functions
SettingsDialog::SettingsDialog(QWidget *parent):
    QDialog(parent),
    m_ui(nullptr),
    m_regex(nullptr),
    m_saveConfigDialog(nullptr)
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
}

void SettingsDialog::verifyIP()
{
    int pos(0);
    QString ip(m_ui->ip->text());
    bool isValid(m_regex->validate(ip,
                                   pos) == QValidator::Acceptable);
    m_ui->connectBtn->setEnabled(isValid);
    m_ui->saveBtn->setEnabled(isValid);
}

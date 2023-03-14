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

#include "settings.hpp"
#include "../view/ui_settings.h"

Settings *Settings::create(QWidget *parent)
{
    Settings *ret(nullptr);
    try
    {
        ret = new Settings(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::Settings;
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);

    ret->m_model = SettingsModel::create(ret);
    if (!ret->m_model)
    {
        delete ret;
        return nullptr;
    }

    ret->m_global = Global::instance();
    if (ret->m_global == nullptr)
    {
        delete ret;
        return nullptr;
    }

    ret->m_settings = ret->m_global->settings();
    ret->m_configs = ret->m_settings["config"].toList();
    QHash<QString, QVariant> state;
    if (ret->m_global->state("settingsState", state))
    {
        if (!ret->m_configs.size())
        {
            ret->m_ui->host->setCurrentIndex(-1);
            return ret;
        }

        ret->updateHostList();
        ret->m_ui->host->setCurrentText(0);
        ret->updateUI(0);
        ret->m_ipAccepted = true;
    }
    else
    {
        ret->updateHostList();
        ret->m_ui->host->setCurrentIndex(state["hostIndex"].toInt());
        ret->m_ui->alias->setText(state["alias"].toString());
        ret->m_ui->ip->setText(state["ip"].toString());
        ret->m_ui->port->setValue(state["port"].toInt());
    }

    ret->m_ui->connectBtn->setEnabled(true);
    ret->m_ui->saveBtn->setEnabled(true);
    ret->m_ui->deleteBtn->setEnabled(true);

    ret->connectHook();
    return ret;
}

Settings::~Settings()
{
    if (m_dirty)
    {
        m_settings["config"] = m_configs;
        m_global->saveSettings(m_settings);
    }

    if (m_accepted)
    {
        QHash<QString, QVariant> state;
        state["hostIndex"] = m_ui->host->currentIndex();
        state["alias"] = m_ui->alias->text();
        state["ip"] = m_ui->ip->text();
        state["port"] = m_ui->port->value();
        m_global->setState("settingsState", state);
    }

    if (m_ui) delete m_ui;
    if (m_model) delete m_model;
}

// private slots
void Settings::onModelDone()
{
    bool hasError(false);
    if (m_model->hasError(hasError))
    {
        m_ui->status->setText("Fail to get \"hasError\"");
        setEnabled(true);
        return;
    }

    if (hasError)
    {
        QString lastError;
        if (m_model->lastError(lastError))
        {
            m_ui->status->setText("Fail to get \"lastError\"");
            setEnabled(true);
            return;
        }

        m_ui->status->setText(lastError);
        setEnabled(true);
        return;
    }

    m_accepted = true;
    m_ui->status->setText("Connected.");
    setEnabled(true);
}

void Settings::onHostCurrentIndexChanged(int_fast32_t index)
{
    if (index == -1) return;
    updateUI(index, false);
}

void Settings::onAliasEditingFinished()
{
    m_accepted = false;
    bool res(checkAllInput());
    m_ui->connectBtn->setEnabled(res);
    m_ui->saveBtn->setEnabled(res);
}

void Settings::onIpEditingFinished()
{
    // m_ipRegex->validate(tmpString, pos) != QValidator::Acceptable
    int_fast32_t pos(0);
    QString text = m_ui->ip->text();
    m_ipAccepted = (m_global->ipRegex()->validate(text, pos) ==
                    QValidator::Acceptable);

    onAliasEditingFinished();
}

void Settings::onDeleteBtnClicked()
{
    auto it = m_configs.begin();
    for (auto i = 0; i < m_ui->host->currentIndex(); ++i)
    {
        ++it;
    }

    m_configs.erase(it);
    m_dirty = true;
    m_ui->deleteBtn->setEnabled(!m_configs.isEmpty());
    updateHostList();
}

void Settings::onSaveBtnClicked()
{
    QHash<QString, QVariant> res;
    res["alias"] = m_ui->alias->text();
    res["ip"] = m_ui->ip->text();
    res["port"] = m_ui->port->value();

    m_configs.append(res);
    updateHostList();
    updateUI(m_configs.size() - 1);
    m_dirty = true;

    if (!m_ui->deleteBtn->isEnabled())
    {
        m_ui->deleteBtn->setEnabled(true);
    }
}

void Settings::onConnectBtnClicked()
{
    setEnabled(false);
    m_accepted = false;
    m_ui->status->setText("Connecting.");
    m_model->startConnect(m_ui->ip->text(), m_ui->port->value());
}

// private member function
Settings::Settings(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_model(nullptr),
    m_dirty(false),
    m_ipAccepted(false),
    m_accepted(false)
{}

void Settings::updateHostList()
{
    m_ui->host->clear();
    for (auto i = 0; i < m_configs.size(); ++i)
    {
        auto out = m_configs.at(i).toHash();
        m_ui->host->addItem(out["alias"].toString());
    }
}

void Settings::updateUI(int_fast32_t index, bool isNotSlot)
{
    auto config = m_configs[index].toHash();
    if (isNotSlot)
    {
        m_ui->host->setCurrentIndex(index);
    }

    m_ui->alias->setText(config["alias"].toString());
    m_ui->ip->setText(config["ip"].toString());
    m_ui->port->setValue(config["port"].toInt());
}

bool Settings::checkAllInput()
{
    return (m_ipAccepted && !m_ui->alias->text().isEmpty());
}

void Settings::connectHook()
{
    connect(m_model,
            &SettingsModel::done,
            this,
            &Settings::onModelDone);

    // ui
    connect(m_ui->host,
            &QComboBox::currentIndexChanged,
            this,
            &Settings::onHostCurrentIndexChanged);

    connect(m_ui->alias,
            &QLineEdit::editingFinished,
            this,
            &Settings::onAliasEditingFinished);

    connect(m_ui->ip,
            &QLineEdit::editingFinished,
            this,
            &Settings::onIpEditingFinished);

    connect(m_ui->deleteBtn,
            &QPushButton::clicked,
            this,
            &Settings::onDeleteBtnClicked);

    connect(m_ui->saveBtn,
            &QPushButton::clicked,
            this,
            &Settings::onSaveBtnClicked);

    connect(m_ui->connectBtn,
            &QPushButton::clicked,
            this,
            &Settings::onConnectBtnClicked);
}

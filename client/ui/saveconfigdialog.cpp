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

#include <new>

#include "QKeyEvent"

#include "saveconfigdialog.hpp"

#include "ui_saveconfigdialog.h"

// public member function
SaveConfigDialog *SaveConfigDialog::create(QHash<QString, SettingsData> *config,
    QWidget *parent)
{
    if (!config) return nullptr;

    SaveConfigDialog *ret(new (std::nothrow) SaveConfigDialog(parent));
    if (!ret) return nullptr;

    ret->m_ui = new (std::nothrow) Ui::SaveConfigDialog;
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->m_config = config;
    return ret;
}

SaveConfigDialog::~SaveConfigDialog()
{
    if (m_ui) delete m_ui;
}

QString SaveConfigDialog::getName() const
{
    return m_ui->name->text();
}

void SaveConfigDialog::reset()
{
    m_ui->name->clear();
    done(0);
}

void SaveConfigDialog::reject()
{
    reset();
}

// protected member function
void SaveConfigDialog::keyPressEvent(QKeyEvent *e)
{
    if (!e)
    {
        QDialog::keyPressEvent(e);
        return;
    }

    int key(e->key());
    switch (key)
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        if (!m_ui->okBtn->isEnabled()) return;
        accept();
        return;
    }
    case Qt::Key_Escape:
    {
        reset();
        return;
    }
    default:
    {
        break;
    }
    }

    QDialog::keyPressEvent(e);
}

// private slots
void SaveConfigDialog::on_name_textChanged(const QString &)
{
    QString name(m_ui->name->text());
    if (name.isEmpty())
    {
        m_ui->okBtn->setEnabled(false);
        m_ui->status->setText("Name is empty!");
        return;
    }

    if (m_config->find(name) != m_config->end())
    {
        m_ui->okBtn->setEnabled(false);
        m_ui->status->setText("Name is duplicate!");
        return;
    }

    m_ui->okBtn->setEnabled(true);
    m_ui->status->setText("Ready");
}

void SaveConfigDialog::on_okBtn_clicked(bool)
{
    emit accepted();
}

void SaveConfigDialog::on_cancelBtn_clicked(bool)
{
    reset();
}

// private member functions
SaveConfigDialog::SaveConfigDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr),
    m_config(nullptr)
{}

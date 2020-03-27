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

#include "inputdialog.hpp"
#include "ui_inputdialog.h"

InputDialog *InputDialog::create(QWidget *parent)
{
    InputDialog *ret(new (std::nothrow) InputDialog(parent));
    if (!ret)
    {
        return nullptr;
    }

    ret->m_ui = new (std::nothrow) Ui::InputDialog();
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->connectHook();

    return ret;
}

InputDialog::~InputDialog()
{
    if (m_ui) delete m_ui;
}

// public member functions
QString InputDialog::getText() const
{
    return m_ui->inputText->text();
}

void InputDialog::reset()
{
    m_ui->inputText->setText("");
    m_ui->selectBtn->setStandardButtons(QDialogButtonBox::Cancel);
}

// private slots
void InputDialog::on_inputText_textChanged(const QString &text)
{
    m_ui->selectBtn->setStandardButtons(text.isEmpty() ?
                                        QDialogButtonBox::StandardButton::Cancel :
                                        QDialogButtonBox::StandardButton::Ok |
                                        QDialogButtonBox::StandardButton::Cancel);
}

void InputDialog::on_inputText_returnPressed()
{
    if (m_ui->inputText->text().isEmpty()) return;
    emit accepted();
}

void InputDialog::on_selectBtn_rejected()
{
    close();
    reset();
}

// private member functions
void InputDialog::connectHook()
{
    // QDialogButtonBox
    connect(m_ui->selectBtn,
            SIGNAL(accepted()),
            this,
            SIGNAL(accepted()));
}

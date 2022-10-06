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

#include "fakeconsole.hpp"
#include "../view/ui_fakeconsole.h"

FakeConsole *FakeConsole::create(QWidget *parent)
{
    FakeConsole *ret(nullptr);

    try
    {
        ret = new FakeConsole(parent);
    }
    catch(...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::FakeConsole;
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_model = FakeConsoleModel::create(ret);
    if (!ret->m_model)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->connectHook();
    return ret;
}

FakeConsole::~FakeConsole()
{
    onStopBtnClicked();
    m_model->wait();

    if (m_ui) delete m_ui;
    if (m_model) delete m_model;
}

// private slots
void FakeConsole::onModelErrorOccurred()
{
    QString err;
    if (m_model->lastError(err))
    {
        err = "Fail to get last error.";
    }

    onModelServerMsg(err + "\n");
    m_ui->startBtn->setEnabled(true);
    m_ui->stopBtn->setEnabled(false);
}

void FakeConsole::onModelServerMsg(const QString &data)
{
    if (data.endsWith('\n'))
        {
#ifdef Q_OS_WIN32
            QByteArray input(data.toUtf8());
            input.replace("\r\n", "\n");
            m_ui->console->insertPlainText(input);
#else
            m_ui->console->insertPlainText(data);
#endif
        }
        else // \r
        {
            QByteArray input(data.toUtf8());
            input.replace("\r", "");
            if (m_prevData.endsWith('\r'))
            {
                QTextCursor cursor = m_ui->console->textCursor();
                cursor.movePosition(QTextCursor::Start);
                cursor.movePosition(QTextCursor::Down,
                                    QTextCursor::MoveAnchor,
                                    m_ui->console->document()->blockCount() - 1);
                cursor.select(QTextCursor::LineUnderCursor);
                cursor.removeSelectedText();
                m_ui->console->setTextCursor(cursor);
            }

            m_ui->console->insertPlainText(input);
        }

        m_ui->console->ensureCursorVisible();
        m_prevData = data;
}

void FakeConsole::onStartBtnClicked()
{
    if (m_model->startConsole())
    {
        onModelServerMsg("Fail to start console.\n");
        return;
    }

    m_ui->startBtn->setEnabled(false);
    m_ui->stopBtn->setEnabled(true);
}

void FakeConsole::onStopBtnClicked()
{
    if (m_model->stopConsole())
    {
        onModelServerMsg("Fail to stop console.\n");
        return;
    }

    m_ui->startBtn->setEnabled(true);
    m_ui->stopBtn->setEnabled(false);
}

// private member functions
FakeConsole::FakeConsole(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_model(nullptr),
    m_prevData("")
{}

void FakeConsole::connectHook()
{
    // model
    connect(m_model,
            &FakeConsoleModel::errorOccurred,
            this,
            &FakeConsole::onModelErrorOccurred);

    connect(m_model,
            &FakeConsoleModel::serverMsg,
            this,
            &FakeConsole::onModelServerMsg);

    // ui
    connect(m_ui->startBtn,
            &QPushButton::clicked,
            this,
            &FakeConsole::onStartBtnClicked);

    connect(m_ui->stopBtn,
            &QPushButton::clicked,
            this,
            &FakeConsole::onStopBtnClicked);

    connect(m_ui->clearBtn,
            &QPushButton::clicked,
            m_ui->console,
            &QPlainTextEdit::clear);
}

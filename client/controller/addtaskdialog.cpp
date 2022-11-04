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

#include "QKeyEvent"
#include "QMessageBox"

#include "addtaskdialog.hpp"
#include "../view/ui_addtaskdialog.h"

// public member functions
AddTaskDialog *AddTaskDialog::create(PendingListModel *model, QWidget *parent)
{
    if (!model) return nullptr;

    AddTaskDialog *ret(nullptr);
    try
    {
        ret = new AddTaskDialog(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::AddTaskDialog;
    }
    catch(...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->m_model = model;

    ret->connectHook();
    return ret;
}

AddTaskDialog::~AddTaskDialog()
{
    if (m_ui) delete m_ui;
}

void AddTaskDialog::accept()
{
    if (!m_ui->closeBtn->isEnabled()) return;
    handleCloseWindow();
    QDialog::accept();
}

void AddTaskDialog::done(int i)
{
    if (!m_ui->closeBtn->isEnabled()) return;
    handleCloseWindow();
    QDialog::done(i);
}

void AddTaskDialog::open()
{
    connect(m_model, &PendingListModel::errorOccurred,
            this, &AddTaskDialog::onModelErrorOccurred);
    checkInputText();
    QDialog::open();
}

void AddTaskDialog::reject()
{
    if (!m_ui->closeBtn->isEnabled()) return;
    handleCloseWindow();
    QDialog::reject();
}

// protected member functions
void AddTaskDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        if (!m_ui->addBtn->isEnabled()) return;
        onAddBtnClicked();
        return;
    }
    case Qt::Key_Escape:
    {
        if (!m_ui->closeBtn->isEnabled()) return;
        reject();
        return;
    }
    default:
    {
        QDialog::keyPressEvent(e);
        break;
    }
    }
}

// private slots
void AddTaskDialog::onModelErrorOccurred()
{
    QString err;
    if (m_model->lastError(err))
    {
        err = "Fail to get last error.";
    }

    QMessageBox::critical(this, "Error", err);
    m_ui->closeBtn->setEnabled(true);
    checkInputText();
}

void AddTaskDialog::onModelAddDone()
{
    uint32_t resID(0);
    QString err;
    if (m_model->resID(resID))
    {
        err = "Fail to get result's ID.";
        QMessageBox::critical(this, "Error", err);
    }
    else
    {
        err = "Success. ID: " + QString::number(resID);
        QMessageBox::information(this, "Success", err);
    }

    m_ui->closeBtn->setEnabled(true);
    checkInputText();
}

void AddTaskDialog::onInputTextChanged(const QString &)
{
    checkInputText();
}

void AddTaskDialog::onAddBtnClicked()
{
    m_ui->addBtn->setEnabled(false);
    std::vector<std::string> args;

    QStringList argList = m_ui->args->toPlainText().split("\n",
                                                          Qt::SkipEmptyParts);

    size_t argSize(argList.size());
    if (argSize)
    {
        args.reserve(argSize);
        for (size_t i = 0; i < argSize; ++i)
        {
            args.push_back(argList.at(i).toUtf8().toStdString());
        }
    }

    m_model->startAdd(m_ui->workDir->text().toStdString(),
                      m_ui->programName->text().toStdString(),
                      args,
                      static_cast<uint32_t>(m_ui->priority->currentIndex()));
}

void AddTaskDialog::onClearBtnClicked()
{
    m_ui->workDir->clear();
    m_ui->programName->clear();
    m_ui->args->clear();

    if (m_ui->addBtn->isEnabled())
        m_ui->addBtn->setEnabled(false);
    m_ui->priority->setCurrentIndex(2);
}

void AddTaskDialog::onCloseBtnClicked()
{
    handleCloseWindow();
    QDialog::reject();
}

// private member functions
AddTaskDialog::AddTaskDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr),
    m_model(nullptr)
{}

void AddTaskDialog::handleCloseWindow()
{
    disconnect(m_model, &PendingListModel::errorOccurred,
               this, &AddTaskDialog::onModelErrorOccurred);
}

void AddTaskDialog::checkInputText()
{
    m_ui->addBtn->setEnabled(!m_ui->workDir->text().isEmpty() &&
                             !m_ui->programName->text().isEmpty());
}

void AddTaskDialog::connectHook()
{
    // model
    connect(m_model, &PendingListModel::addDone,
            this, &AddTaskDialog::onModelAddDone);

    // ui
    connect(m_ui->workDir, &QLineEdit::textChanged,
            this, &AddTaskDialog::onInputTextChanged);

    connect(m_ui->programName, &QLineEdit::textChanged,
            this, &AddTaskDialog::onInputTextChanged);

    connect(m_ui->addBtn, &QPushButton::clicked,
            this, &AddTaskDialog::onAddBtnClicked);

    connect(m_ui->clearBtn, &QPushButton::clicked,
            this, &AddTaskDialog::onClearBtnClicked);

    connect(m_ui->closeBtn, &QPushButton::clicked,
            this, &AddTaskDialog::onCloseBtnClicked);
}

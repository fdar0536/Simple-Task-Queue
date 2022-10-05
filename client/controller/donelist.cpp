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

#include "QMessageBox"

#include "donelist.hpp"
#include "../view/ui_donelist.h"

DoneList *DoneList::create(QWidget *parent)
{
    DoneList *ret(nullptr);

    try
    {
        ret = new DoneList(parent);
    }
    catch (...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::DoneList;
    }
    catch (...)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);

    ret->m_model = DoneListModel::create(ret);
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

    if (ret->m_global->taskDetailsDialog(&ret->m_taskDetailsDialog))
    {
        delete ret;
        return nullptr;
    }

    ret->connectHook();
    ret->onRefreshBtnClicked();
    return ret;
}

DoneList::~DoneList()
{
    if (m_ui) delete m_ui;
    if (m_model) delete m_model;

    m_global->freeTaskDetailsDialog();
}

// private slots
void DoneList::onModelErrorOccurred()
{
    QString err;
    if (m_model->lastError(err))
    {
        err = "Fail to get last error.";
    }

    QMessageBox::critical(this, "Error", err);
    m_ui->status->setText(err);
    setEnabled(true);
}

void DoneList::onModelListDone()
{
    QString err;
    setEnabled(true);

    if (m_model->doneList(m_list))
    {
        err = "Fail to get pending list.";
        QMessageBox::critical(this, "Error", err);
        m_ui->status->setText(err);
        return;
    }

    m_ui->doneList->clear();
    if (m_list.size())
    {
        for (auto it = m_list.begin(); it != m_list.end(); ++it)
        {
            m_ui->doneList->addItem(QString::number(*it));
        }
    }

    m_ui->status->setText("Done");
}

void DoneList::onModelDetailsDone()
{
    setEnabled(true);
    TaskDetails taskDetails;
    QString err;
    if (m_model->taskDetails(taskDetails))
    {
        err = "Fail to get task details.";
        QMessageBox::critical(this, "Error", err);
        m_ui->status->setText(err);
        return;
    }

    m_taskDetailsDialog->openDialog(taskDetails, true);
}

void DoneList::onDoneListItemActivated(QListWidgetItem *item)
{
    if (!item) return;

    m_model->startDetails(item->text().toUInt());
    setEnabled(false);
}

void DoneList::onRefreshBtnClicked()
{
    setEnabled(false);
    m_ui->status->setText("Fetching");
    m_model->startList();
}

void DoneList::onClearBtnClicked()
{
    setEnabled(false);
    m_ui->status->setText("Clearing");
    m_model->startClear();
}

// private member functions
DoneList::DoneList(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_model(nullptr),
    m_global(nullptr),
    m_taskDetailsDialog(nullptr)
{
    m_list.reserve(64);
}

void DoneList::connectHook()
{
    // model
    connect(m_model,
            &DoneListModel::errorOccurred,
            this,
            &DoneList::onModelErrorOccurred);

    connect(m_model,
            &DoneListModel::listDone,
            this,
            &DoneList::onModelListDone);

    connect(m_model,
            &DoneListModel::detailsDone,
            this,
            &DoneList::onModelDetailsDone);

    // ui
    connect(m_ui->doneList,
            &QListWidget::itemActivated,
            this,
            &DoneList::onDoneListItemActivated);

    connect(m_ui->refreshBtn,
            &QPushButton::clicked,
            this,
            &DoneList::onRefreshBtnClicked);

    connect(m_ui->clearBtn,
            &QPushButton::clicked,
            this,
            &DoneList::onClearBtnClicked);
}

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

#pragma once

#include "QListWidgetItem"
#include "QWidget"

#include "addtaskdialog.hpp"
#include "model/global.hpp"
#include "model/pendinglistmodel.hpp"

namespace Ui
{
class PendingList;
}

class PendingList : public QWidget
{
    Q_OBJECT

public:

    static PendingList *create(QWidget * = nullptr);

    ~PendingList();

private slots:

    void onModelErrorOccurred();

    void onModelDetailsDone();

    void onModelListDone();

    void onAddTaskDialogClosed();

    // ui
    void onCurrentDetailClicked();

    void onPendingItemActivated(QListWidgetItem *item);

    void onAddBtnClicked();

    void onDeleteBtnClicked();

    void onRefreshBtnClicked();

    void onStartBtnClicked();

    void onStopBtnClicked();

private:

    PendingList(QWidget * = nullptr);

    Ui::PendingList *m_ui;

    PendingListModel *m_model;

    AddTaskDialog *m_addTaskDialog;

    TaskDetails m_taskDetails;

    std::vector<uint32_t> m_list;

    std::shared_ptr<Global> m_global;

    TaskDetailsDialog *m_taskDetailsDialog;

    void connectHook();
};


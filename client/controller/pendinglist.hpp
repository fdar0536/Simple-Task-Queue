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


#pragma once

#include "QDialog"

#include "model/taskdetails.hpp"

namespace Ui {
class TaskDetailsDialog;
}

class TaskDetailsDialog : public QDialog
{

    Q_OBJECT

public:

    TaskDetailsDialog(QWidget * = nullptr);

    static TaskDetailsDialog *create(QWidget * = nullptr);

    ~TaskDetailsDialog();

    void openDialog(TaskDetails &, bool);

protected:

    void keyPressEvent(QKeyEvent *) override;

private:

    Ui::TaskDetailsDialog *m_ui;

    void connectHook();
};


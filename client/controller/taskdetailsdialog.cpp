#include "QKeyEvent"

#include "taskdetailsdialog.hpp"
#include "../view/ui_taskdetailsdialog.h"

static std::vector<QString> priorityString =
{
    "High", "Abrove nornal", "Normal", "Below nornal", "Low"
};

TaskDetailsDialog::TaskDetailsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr)
{
}

std::shared_ptr<TaskDetailsDialog> TaskDetailsDialog::create()
{
    std::shared_ptr<TaskDetailsDialog> ret(nullptr);

    try
    {
        ret = std::make_shared<TaskDetailsDialog>(nullptr);
    }
    catch(...)
    {
        return nullptr;
    }

    try
    {
        ret->m_ui = new Ui::TaskDetailsDialog;
    }
    catch(...)
    {
        return nullptr;
    }

    ret->m_ui->setupUi(ret.get());
    return ret;
}

TaskDetailsDialog::~TaskDetailsDialog()
{
    if (m_ui) delete m_ui;
}

void TaskDetailsDialog::openDialog(TaskDetails &task, bool isExitCodeValid)
{
    m_ui->workDir->setText(QString::fromStdString(task.workDir));
    m_ui->programName->setText(QString::fromStdString(task.programName));
    m_ui->args->clear();

    size_t size = task.args.size();
    QString args = "";
    if (size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            args += QString::fromStdString(task.args.at(i));
            args += '\n';
        }
    }

    if (isExitCodeValid)
    {
        m_ui->exitCode->setText(QString::number(task.exitCode));
        m_ui->priority->setText("Priority is not available.");
        m_ui->dlBtn->setEnabled(true);
    }
    else
    {
        m_ui->exitCode->setText("Exit code is not available.");
        m_ui->priority->setText(priorityString.at(task.priority));
        m_ui->dlBtn->setEnabled(false);
    }

    open();
}

// protected member functions
void TaskDetailsDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
    {
        reject();
        break;
    }
    default:
    {
        QDialog::keyPressEvent(e);
        break;
    }
    }
}

// private slots
void TaskDetailsDialog::on_closeBtn_clicked()
{
    reject();
}

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

TaskDetailsDialog *TaskDetailsDialog::create(QWidget *parent)
{
    TaskDetailsDialog *ret(nullptr);

    try
    {
        ret = new TaskDetailsDialog(parent);
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
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->connectHook();
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

    QDialog::open();
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

// private member functions
void TaskDetailsDialog::connectHook()
{
    connect(m_ui->closeBtn,
            &QPushButton::clicked,
            this,
            &TaskDetailsDialog::reject);
}

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
    connect(ret->m_model, &PendingListModel::addDone,
            ret, &AddTaskDialog::onModelAddDone);

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
        on_addBtn_clicked();
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

void AddTaskDialog::on_workDir_textChanged(const QString &)
{
    checkInputText();
}

void AddTaskDialog::on_programName_textChanged(const QString &)
{
    checkInputText();
}

void AddTaskDialog::on_addBtn_clicked()
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
            args.push_back(argList.at(i).toStdString());
        }
    }

    m_model->startAdd(m_ui->workDir->text().toStdString(),
                      m_ui->programName->text().toStdString(),
                      args,
                      static_cast<uint32_t>(m_ui->priority->currentIndex()));
}

void AddTaskDialog::on_clearBtn_clicked()
{
    m_ui->workDir->clear();
    m_ui->programName->clear();
    m_ui->args->clear();

    if (m_ui->addBtn->isEnabled())
        m_ui->addBtn->setEnabled(false);
    m_ui->priority->setCurrentIndex(2);
}

void AddTaskDialog::on_closeBtn_clicked()
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

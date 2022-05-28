#include <new>

#include "QKeyEvent"

#include "saveconfigdialog.hpp"

#include "ui_saveconfigdialog.h"

// public member function
SaveConfigDialog *SaveConfigDialog::create(QHash<QString, SettingsData> *config,
    QWidget *parent)
{
    if (!config) return nullptr;

    SaveConfigDialog *ret(new (std::nothrow) SaveConfigDialog(parent));
    if (!ret) return nullptr;

    ret->m_ui = new (std::nothrow) Ui::SaveConfigDialog;
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->m_config = config;
    return ret;
}

SaveConfigDialog::~SaveConfigDialog()
{
    if (m_ui) delete m_ui;
}

QString SaveConfigDialog::getName() const
{
    return m_ui->name->text();
}

void SaveConfigDialog::reset()
{
    m_ui->name->clear();
    close();
}

void SaveConfigDialog::reject()
{
    reset();
}

// protected member function
void SaveConfigDialog::keyPressEvent(QKeyEvent *e)
{
    if (!e)
    {
        QDialog::keyPressEvent(e);
        return;
    }

    int key(e->key());
    switch (key)
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
        if (!m_ui->okBtn->isEnabled()) return;
        emit accepted();
        return;
    }
    case Qt::Key_Escape:
    {
        reset();
        return;
    }
    default:
    {
        break;
    }
    }

    QDialog::keyPressEvent(e);
}

// private slots
void SaveConfigDialog::on_name_textChanged(const QString &)
{
    QString name(m_ui->name->text());
    if (name.isEmpty())
    {
        m_ui->okBtn->setEnabled(false);
        m_ui->status->setText("Name is empty!");
        return;
    }

    if (m_config->find(name) != m_config->end())
    {
        m_ui->okBtn->setEnabled(false);
        m_ui->status->setText("Name is duplicate!");
        return;
    }

    m_ui->okBtn->setEnabled(true);
    m_ui->status->setText("Ready");
}

void SaveConfigDialog::on_okBtn_clicked(bool)
{
    emit accepted();
}

void SaveConfigDialog::on_cancelBtn_clicked(bool)
{
    reset();
}

// private member functions
SaveConfigDialog::SaveConfigDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr),
    m_config(nullptr)
{}

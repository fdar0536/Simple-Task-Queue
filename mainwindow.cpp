/*
 * Simple Task Queue
 * Copyright (c) 2020 fdar0536
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

#include <new>

#include "QMessageBox"
#include "QInputDialog"
#include "QCloseEvent"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "config.h"

MainWindow *MainWindow::create(QWidget *parent)
{
    MainWindow *ret(new (std::nothrow) MainWindow(parent));
    if (!ret)
    {
        return nullptr;
    }
    ret->m_ui = new (std::nothrow) Ui::MainWindow();
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    ret->m_inputDialog = InputDialog::create(ret);
    if (!ret->m_inputDialog)
    {
        delete ret;
        return nullptr;
    }

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        ret->m_icon = new (std::nothrow) QSystemTrayIcon(QIcon(":/STQ.ico"), ret);
        if (!ret->m_icon)
        {
            delete ret;
            return nullptr;
        }

        ret->m_icon->show();

        //iconContextMenu
        ret->m_iconContextMenu = new (std::nothrow) QMenu(ret);
        if (!ret->m_iconContextMenu)
        {
            delete ret;
            return nullptr;
        }

        ret->m_iconAction1 = new (std::nothrow) QAction("Show", ret);
        if (!ret->m_iconAction1)
        {
            delete ret;
            return nullptr;
        }

        ret->m_iconAction2 = new (std::nothrow) QAction("Exit", ret);
        if (!ret->m_iconAction2)
        {
            delete ret;
            return nullptr;
        }

        ret->m_iconContextMenu->addAction(ret->m_iconAction1);
        ret->m_iconContextMenu->addAction(ret->m_iconAction2);
        ret->m_icon->setContextMenu(ret->m_iconContextMenu);
    }

    QString title(STQ_NAME);
    title += " - ";
    ret->setWindowTitle(title + STQ_VERSION);

    ret->connectHook();

    return ret;
}

MainWindow::~MainWindow()
{
    if (m_inputDialog) delete m_inputDialog;

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (m_icon) delete m_icon;
        if (m_iconContextMenu) delete m_iconContextMenu;
        if (m_iconAction1) delete m_iconAction1;
        if (m_iconAction2) delete m_iconAction2;
    }

    for (int i = 0; i < m_ui->taskTab->count(); ++i)
    {
        TaskTab *widget(qobject_cast<TaskTab *>(m_ui->taskTab->widget(i)));
        delete widget;
    }

    if (m_ui) delete m_ui;
}

// private slots
// actions
void MainWindow::on_actionAdd_triggered()
{
    m_inputDialog->open();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::programExit()
{
    QMessageBox::StandardButton
            res = QMessageBox::question(this,
                                        tr("Warning"),
                                        tr("Are you sure to exit? "
                                           "All unfinished tasks will be killed."),
                                        QMessageBox::Cancel |
                                        QMessageBox::No |
                                        QMessageBox::Yes);

    if (res == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

// InputDialog
void MainWindow::onInputAccepted()
{
    TaskTab *toBeInserted(TaskTab::create(this));
    if (!toBeInserted)
    {
        QMessageBox::critical(this, tr("Error"), tr("Fail to insert item!"));
        return;
    }

    connect(toBeInserted,
            SIGNAL(renameRequried(TaskTab *, QString &)),
            this,
            SLOT(onTabRenameRequried(TaskTab *, QString &)));

    QString title(m_inputDialog->getText());
    m_ui->taskTab->insertTab(m_ui->taskTab->count(),
                             toBeInserted,
                             title);

    onInputRejected();
}

void MainWindow::onInputRejected()
{
    m_inputDialog->close();
    m_inputDialog->reset();
}

// TaskTab
void MainWindow::on_taskTab_tabCloseRequested(int index)
{
    QMessageBox::StandardButton
            res = QMessageBox::question(this,
                                        tr("Warning"),
                                        tr("Are you sure to close? "
                                           "All unfinished tasks will be killed."),
                                        QMessageBox::Cancel |
                                        QMessageBox::No |
                                        QMessageBox::Yes);

    if (res == QMessageBox::Yes)
    {
        TaskTab *widget(qobject_cast<TaskTab *>(m_ui->taskTab->widget(index)));
        m_ui->taskTab->removeTab(index);
        delete widget;
    }
}

void
MainWindow::onTabRenameRequried(TaskTab *tab, QString &text)
{
    m_ui->taskTab->setTabText(m_ui->taskTab->indexOf(tab), text);
}

// Tray Icon
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick)
    {
        this->show();
    }
}

void MainWindow::iconAction1Triggered()
{
    this->show();
}

// private member function
void MainWindow::connectHook()
{
    // actions
    connect(m_ui->actionExit,
            SIGNAL(triggered()),
            this,
            SLOT(programExit()));

    // InputDialog
    connect(m_inputDialog,
            SIGNAL(accepted()),
            this,
            SLOT(onInputAccepted()));

    connect(m_inputDialog,
            SIGNAL(rejected()),
            this,
            SLOT(onInputRejected()));

    //trayIcon
    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect(m_icon,
                SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,
                SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        connect(m_iconAction1,
                SIGNAL(triggered()),
                this,
                SLOT(iconAction1Triggered()));

        connect(m_iconAction2,
                SIGNAL(triggered()),
                this,
                SLOT(programExit()));
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    e->ignore();
    this->hide();
}

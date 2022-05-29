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

#include <new>

#include "QMessageBox"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr),
    m_logger(nullptr)
{}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
    if (m_logger) delete m_logger;
    if (m_settingsDialog) delete m_settingsDialog;
}

// public member functions
uint8_t MainWindow::init(MainWindow *w)
{
    if (!w) return 1;

    w->m_ui = new (std::nothrow) Ui::MainWindow;
    if (!w->m_ui) return 1;

    w->m_ui->setupUi(w);

    w->m_logger = LoggerDialog::create(w);
    if (!w->m_logger)
    {
        return 1;
    }

    w->m_settingsDialog = SettingsDialog::create(w);
    if (!w->m_settingsDialog)
    {
        return 1;
    }

    return 0;
}

// public slots
void MainWindow::on_actionSettings_triggered()
{
    m_settingsDialog->open();
}

// private slots
void MainWindow::on_actionLog_triggered()
{
    m_logger->open();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

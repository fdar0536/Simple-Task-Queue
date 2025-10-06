/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
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

#include "fmt/format.h"

#include "QDebug"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

namespace View
{

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_ui(nullptr),
    m_configForm(nullptr)
{}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
}

u8 MainWindow::init()
{
    if (!m_ui) m_ui = new (std::nothrow) Ui::MainWindow;
    if (!m_ui)
    {
        qCritical(
            fmt::format("{}:{} Fail to allocate memory",
                        __FILE__, __LINE__).c_str());
        return 1;
    }

    m_ui->setupUi(this);

    m_configForm = new(std::nothrow) ConfigForm(this);
    if (!m_configForm->init())
    {
        qCritical(
            fmt::format("{}:{} ConfigForm init failed",
                        __FILE__, __LINE__).c_str());
        return 1;
    }

    connectHook();
    return 0;
}

// private member functions
void MainWindow::connectHook()
{}

} // namespace View

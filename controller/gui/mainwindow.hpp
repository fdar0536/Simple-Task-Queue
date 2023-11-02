/* Simple Task Queue
 * Copyright (c) 2023 fdar0536
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

#ifndef _CONTROLLER_GUI_MAINWINDOW_HPP_
#define _CONTROLLER_GUI_MAINWINDOW_HPP_

#include "QMainWindow"
#include "QMessageBox"
#include "QSystemTrayIcon"
#include "spdlog/spdlog.h"

namespace Ui
{

class MainWindow;

}

namespace Controller
{

namespace GUI
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget * = nullptr);

    ~MainWindow();

    uint_fast8_t init();

protected:

    virtual void closeEvent(QCloseEvent *) override;

private slots:
    void onIconActivated(QSystemTrayIcon::ActivationReason);

    // action
    void onServerConfigActionTriggered(bool);

    void onClientConfigActionTriggered(bool);

    void onLogActionTriggered(bool);

    void onAboutQtActionTriggered(bool);

    void onExitActionTriggered(bool);

    void onShowActionTriggered(bool);

private:

    Ui::MainWindow *m_ui;

    std::shared_ptr<spdlog::logger> m_defaultLogger;

    QSystemTrayIcon *m_icon;

    QMenu *m_iconContextMenu;

    QAction *m_showAction;

    void sqliteInit();

    void spdlogInit();

    uint_fast8_t trayIconInit();

    void trayIconFin();

    void connectHook();

    template<class T>
    void updateCentralWidget()
    {
        T *newWidget = new (std::nothrow) T(this);
        if (!newWidget)
        {
            QMessageBox::critical(this, tr("Error"), tr("Fail to allocate memory"));
            spdlog::error("{}:{} Fail to allocate memory",
                          __FILE__, __LINE__);
            return;
        }

        if (newWidget->init())
        {
            delete newWidget;
            QMessageBox::critical(this, tr("Error"), tr("Fail to initialize widget"));
            spdlog::error("{}:{} Fail to initialize widget",
                          __FILE__, __LINE__);
            return;
        }

        QWidget *toRemove = takeCentralWidget();
        delete toRemove;
        setCentralWidget(newWidget);
    }

}; // end class MainWindow

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_MAINWINDOW_HPP_

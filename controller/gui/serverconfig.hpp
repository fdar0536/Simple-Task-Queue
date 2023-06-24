/*
 * Simple Task Queue
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

#ifndef _CONTROLLER_GUI_SERVERCONFIG_HPP_
#define _CONTROLLER_GUI_SERVERCONFIG_HPP_

#include "QWidget"

#include "waitforinit.hpp"

namespace Ui
{

class ServerConfig;

}

namespace Controller
{

namespace GUI
{

class ServerConfig : public QWidget
{
    Q_OBJECT

public:

    ServerConfig(QWidget * = nullptr);

    ~ServerConfig();

    uint_fast8_t init();

private slots:

    void onWaitForInitDone();

    void onAutoStartClicked(bool);

    void onServerEditFinished();

    void onClearClicked(bool);

    void onStartClicked(bool);

    void onStopClicked(bool);

private:

    Ui::ServerConfig *m_ui;

    WaitForInit *m_waitForInit;

    void connectHook();

}; // end class ServerConfig

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_SERVERCONFIG_HPP_

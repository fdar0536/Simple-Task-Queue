/* This file is modified from spdlog
 * Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 *
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

#ifndef _CONTROLLER_GUI_CONFIGCONFIG_HPP_
#define _CONTROLLER_GUI_CONFIGCONFIG_HPP_

#include "QWidget"

#include "clientconfigthread.hpp"

class QListWidgetItem;

namespace Ui
{

class ClientConfig;

}

namespace Controller
{

namespace GUI
{

class ClientConfig : public QWidget
{
    Q_OBJECT

public:

    ClientConfig(QWidget * = nullptr);

    ~ClientConfig();

    uint_fast8_t init();

private slots:

    // ui
    void onUseRemoteStateChanged(int);

    void onRemoteListItemActivated(QListWidgetItem *);

    void onRemoveClicked(bool);

    void onSaveClicked(bool);

    void onClearClicked(bool);

    void onConnectClicked(bool);

    // threads
    void onParseConfigDone();

    void onConnnectDone(bool);

private:

    Ui::ClientConfig *m_ui;

    ClientConfigThread *m_thread;

    void connectHook();

    bool addItemIntoRemoteList(bool notOverride);

}; // end class ClientConfig

} // end namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_CONFIGCONFIG_HPP_

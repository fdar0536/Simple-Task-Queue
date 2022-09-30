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

#pragma once

#include "QWidget"

#include "model/global.hpp"
#include "model/settingsmodel.hpp"

namespace Ui
{
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:

    static Settings *create(QWidget * = nullptr);

    ~Settings();

private slots:

    void onModelDone();

    // ui
    void onHostCurrentIndexChanged(int index);

    void onAliasEditingFinished();

    void onIpEditingFinished();

    void onDeleteBtnClicked();

    void onSaveBtnClicked();

    void onConnectBtnClicked();

private:

    Settings(QWidget * = nullptr);

    Ui::Settings *m_ui;

    std::shared_ptr<Global> m_global;

    SettingsModel *m_model;

    QHash<QString, QVariant> m_settings;

    QList<QVariant> m_configs;

    bool m_dirty;

    bool m_ipAccepted;

    bool m_accepted;

    void updateHostList();

    void updateUI(int, bool = true);

    bool checkAllInput();

    void connectHook();

};

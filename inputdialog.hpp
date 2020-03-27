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

#pragma once

#include "QDialog"
#include "QDialogButtonBox"

namespace Ui
{
    class InputDialog;
}

class InputDialog : public QDialog
{

    Q_OBJECT

public:

    static InputDialog *create(QWidget *parent = nullptr);

    ~InputDialog();

    QString getText() const;

    void reset();

private slots:

    void on_inputText_textChanged(const QString &);

    void on_inputText_returnPressed();

    void on_selectBtn_rejected();

private:

    explicit InputDialog(QWidget *parent = nullptr) :
        QDialog(parent),
        m_ui(nullptr)
    {}

    InputDialog() = delete;

    InputDialog(const InputDialog &) = delete;

    InputDialog &operator=(InputDialog) = delete;

    Ui::InputDialog *m_ui;

    void connectHook();
};

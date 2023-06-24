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

#ifndef _CONTROLLER_GUI_QUEUELIST_HPP_
#define _CONTROLLER_GUI_QUEUELIST_HPP_

#include <atomic>

#include "QMap"
#include "QThread"

class QJSValue;

namespace Controller
{

namespace GUI
{

class QueueList : public QThread
{
    Q_OBJECT

public:

    QueueList(QObject * = nullptr);

    ~QueueList();

    Q_INVOKABLE bool init();

    Q_INVOKABLE void startCreate(const QString &);

    Q_INVOKABLE void startListQueue();

    QJSValue

    void run() override;

signals:

    void CreateDone(bool);

    void ListDone(bool);

    void DeleteDone(bool);

    void RenameDone(bool);

    void GetQueueDone(bool);

private:

}; // end class ClientConfig

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_QUEUELIST_HPP_

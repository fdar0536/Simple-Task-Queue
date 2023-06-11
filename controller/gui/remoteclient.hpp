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

#ifndef _CONTROLLER_GUI_REMOTECLIENT_HPP_
#define _CONTROLLER_GUI_REMOTECLIENT_HPP_

#include <atomic>

#include "QMap"
#include "QThread"

class QJSValue;

namespace Controller
{

namespace GUI
{

class RemoteClient : public QThread
{
    Q_OBJECT

    Q_PROPERTY(QString name      READ name      CONSTANT)
    Q_PROPERTY(QString ip        READ ip        CONSTANT)
    Q_PROPERTY(int     port      READ port      CONSTANT)
    Q_PROPERTY(int     dataPages READ dataPages CONSTANT)
    Q_PROPERTY(int     pageIndex READ pageIndex CONSTANT)
    Q_PROPERTY(bool    isNoData  READ isNoData  CONSTANT)

public:

    RemoteClient(QObject * = nullptr);

    ~RemoteClient();

    Q_INVOKABLE bool init();

    Q_INVOKABLE int logLevel() const;

    Q_INVOKABLE int setLogLevel(int);

    QString name() const;

    QString ip() const;

    int port() const;

    int dataPages() const;

    int pageIndex() const;

    bool isNoData() const;

    Q_INVOKABLE bool saveSetting(const QString &, const QString &, const int);

    Q_INVOKABLE void updateData();

    Q_INVOKABLE QJSValue data();

    Q_INVOKABLE void setLastPage(int, int);

    Q_INVOKABLE void deleteData(const QString &);

    void run() override;

signals:

    void InitDone();

    void ServerConnected();

private:

    void initImpl();

    void connectToServerImpl();

    typedef void (RemoteClient::*Handler)();

    typedef enum Mode
    {
        INIT, CONNECT
    } Mode;

    Handler m_handler[2] =
    {
        &RemoteClient::initImpl,
        &RemoteClient::connectToServerImpl
    };

    std::atomic<Mode> m_mode;

    std::atomic<bool> m_isRunning;

    QString m_dataName;

    QMap<QString, QVariant> m_data;

    uint_fast8_t dataInternal();

}; // end class ClientConfig

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_REMOTECLIENT_HPP_

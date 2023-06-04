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

#ifndef _CONTROLLER_GUI_CLIENTCONFIG_HPP_
#define _CONTROLLER_GUI_CLIENTCONFIG_HPP_

#include <atomic>

#include "QMap"
#include "QThread"

namespace Controller
{

namespace GUI
{

class ClientConfig : public QThread
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString ip   READ ip   CONSTANT)
    Q_PROPERTY(int     port READ port CONSTANT)

public:

    ClientConfig(QObject * = nullptr);

    ~ClientConfig();

    Q_INVOKABLE bool init();

    Q_INVOKABLE int logLevel() const;

    Q_INVOKABLE int setLogLevel(int);

    QString name();

    QString ip();

    int port();

    Q_INVOKABLE bool saveSetting(const QString &, const QString &, const int);

    Q_INVOKABLE void updateData();

    void run() override;

signals:

    void InitDone();

    void ServerConnected();

private:

    std::atomic<bool> m_isInit;

    void initImpl();

    void connectToServerImpl();

    typedef void (ClientConfig::*Handler)();

    typedef enum Mode
    {
        INIT, CONNECT
    } Mode;

    Handler m_handler[2] =
    {
        &ClientConfig::initImpl,
        &ClientConfig::connectToServerImpl
    };

    std::atomic<Mode> m_mode;

    std::atomic<bool> m_isRunning;

    QString m_dataName;

    QMap<QString, QVariant> m_data;
}; // end class ClientConfig

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_CLIENTCONFIG_HPP_

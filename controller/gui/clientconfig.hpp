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

#include "QAbstractItemModel"

#include "clientconfigthread.hpp"

namespace Controller
{

namespace GUI
{

class ClientConfig : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(int dataCount READ dataCount CONSTANT)

public:

    ClientConfig(QObject * = nullptr);

    ~ClientConfig();

    int dataCount() const;

    Q_INVOKABLE bool init();

    Q_INVOKABLE int logLevel() const;

    Q_INVOKABLE int setLogLevel(int);

    Q_INVOKABLE QString name(int);

    Q_INVOKABLE QString ip(int);

    Q_INVOKABLE int port(int);

    Q_INVOKABLE bool saveSetting(const QString &, const QString &, const int);

    // pure virtual functions
    Q_INVOKABLE int rowCount(const QModelIndex & = QModelIndex()) const override;

    Q_INVOKABLE int columnCount(const QModelIndex & = QModelIndex()) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override;

    QModelIndex index(int, int, const QModelIndex & = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &) const override;

signals:

    void InitDone();

    void ServerConnected();

private slots:

    void onThreadInitDone(const QList<QVariant> &);

private:

    enum ConfigRoles
    {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    std::atomic<bool> m_isInit;

    ClientConfigThread *m_thread;

    QList<QVariant> m_data;
}; // end class ClientConfig

} // namespace GUI

} // end namespace Controller

#endif // _CONTROLLER_GUI_CLIENTCONFIG_HPP_

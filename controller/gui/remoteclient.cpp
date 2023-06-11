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

#include "QCoreApplication"
#include "QSettings"
#include "controller/global/init.hpp"
#include "model/utils.hpp"
#include "global.hpp"
#include "remoteclient.hpp"

namespace Controller
{

namespace GUI
{

#define COUNT_PER_PAGE 5
#define DATA_INDEX(x, y) x * COUNT_PER_PAGE + y
#define LAST_INDEX DATA_INDEX(_state.lastPage, \
                              _state.lastDataIndex)

#define IS_NOT_VALID_INDEX(x) ((x + 1) > _state.dataCache.count())

typedef struct _STATE
{
    QHash<QString, QVariant> data;
    int lastPage = 0;
    int lastDataIndex = 0;
    int totalPages = 1;
    bool dataIsDirty = false;
    QList<QString> dataCache;
} _STATE;

static _STATE _state;
static std::atomic<bool> isInit;

RemoteClient::RemoteClient(QObject *parent) :
    QThread(parent)
{
    isInit.store(false, std::memory_order_relaxed);
    m_isRunning.store(false, std::memory_order_relaxed);
    updateData();
}

RemoteClient::~RemoteClient()
{
    if (!_state.data.isEmpty())
    {
        QSettings s;
        s.setValue("Server List", _state.data);
    }
}

bool RemoteClient::init()
{
    if (isInit.load(std::memory_order_relaxed))
    {
        emit InitDone();
        return true;
    }

    m_mode.store(INIT, std::memory_order_relaxed);
    start();
    return true;
}

int RemoteClient::logLevel() const
{
    return static_cast<int>(Controller::Global::config.logLevel());
}

int RemoteClient::setLogLevel(int in)
{
    if (in < 0 || in > 6)
    {
        spdlog::error(
            "{}:{} Invalid level, use default value", __FILE__, __LINE__);
        in = 2;
    }

    Controller::Global::config.setLogLevel(
        static_cast<spdlog::level::level_enum>(in));

    return in;
}

QString RemoteClient::name() const
{
    return m_dataName;
}

QString RemoteClient::ip() const
{
    return m_data["ip"].toString();
}

int RemoteClient::port() const
{
    return m_data["port"].toInt();
}

bool RemoteClient::hasPrevPage() const
{
    return _state.lastPage; // if last page == 0, then here has no prev page
}

bool RemoteClient::prevPage()
{
    if (!hasPrevPage())
    {
        return false;
    }

    --_state.lastPage;
    _state.lastDataIndex = 0;

    if (dataInternal())
    {
        m_dataName = "";
        m_data.clear();
        return false;
    }

    return true;
}

bool RemoteClient::hasNextPage() const
{
    return (_state.lastPage + 1) < _state.totalPages;
}

bool RemoteClient::nextPage()
{
    if (!hasNextPage())
    {
        return false;
    }

    ++_state.lastPage;
    _state.lastDataIndex = 0;

    if (dataInternal())
    {
        m_dataName = "";
        m_data.clear();
        return false;
    }

    return true;
}

bool RemoteClient::saveSetting(const QString &name,
                               const QString &ip,
                               const int port)
{
    if (Model::Utils::verifyIP(ip.toUtf8().toStdString()))
    {
        return false;
    }

    if (port < 0 || port > 65535)
    {
        return false;
    }

    QMap<QString, QVariant> map;
    map["ip"] = ip;
    map["port"] = port;

    _state.data[name] = map;
    _state.dataIsDirty = true;
    return true;
}

void RemoteClient::updateData()
{
    if (_state.dataIsDirty)
    {
        _state.dataCache.clear();
        _state.dataIsDirty = false;
        _state.lastPage = 0;
        _state.lastDataIndex = 0;
        _state.totalPages = 1;

        _state.dataCache.reserve(_state.data.count());
        for (auto it = _state.data.begin(); it != _state.data.end(); ++it)
        {
            _state.dataCache.append(it.key());
        }

        _state.totalPages = _state.data.count() / COUNT_PER_PAGE;
        if ((_state.data.count() % COUNT_PER_PAGE) > 0)
        {
            ++_state.totalPages;
        }
    }

    if (dataInternal())
    {
        m_dataName = "";
        m_data.clear();
        return;
    }
}

QJSValue RemoteClient::data()
{

    if (_state.dataCache.length() <= 0)
    {
        return Controller::Global::guiGlobal.engine()->newArray(0);
    }

    int startIndex = _state.lastPage * COUNT_PER_PAGE;
    int dataCount = _state.dataCache.length() - startIndex;
    if (dataCount <= 0)
    {
        _state.dataIsDirty = true;
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid index");
        return Controller::Global::guiGlobal.engine()->newArray(0);
    }

    if (dataCount > 5) dataCount = 5;

    QJSValue ret = Controller::Global::guiGlobal.engine()->newArray(dataCount);
    for (int i = 0, j = startIndex; i < dataCount; ++i, ++j)
    {
        QJSValue toInsert = Controller::Global::guiGlobal.engine()->newObject();
        toInsert.setProperty("name", _state.dataCache.at(j));
        ret.setProperty(i, toInsert);
    }

    return ret;
}

bool RemoteClient::setLastDataIndex(int index)
{
    if (index >= COUNT_PER_PAGE || index < 0)
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid index");
        return false;
    }

    _state.lastDataIndex = index;
    if (dataInternal())
    {
        m_dataName = "";
        m_data.clear();
        return false;
    }

    return true;
}

void RemoteClient::deleteData(const QString &name)
{
    QHash<QString, QVariant>::const_iterator it = _state.data.find(name);
    if (it == _state.data.end())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid name");
        return;
    }

    _state.data.erase(it);
    _state.dataIsDirty = true;
}

void RemoteClient::run()
{
    m_isRunning.store(true, std::memory_order_relaxed);
    (this->*m_handler[m_mode])();
    m_isRunning.store(false, std::memory_order_relaxed);
}

// private member functions
void RemoteClient::initImpl()
{
    // verify settings
    QList<QString> keys = {"ip", "port"};
    QList<int> values = {QMetaType::QString, QMetaType::Int};
    QMap<QString, QVariant> map;
    QSettings settings;
    QHash<QString, QVariant> data = settings.value("Server List", QHash<QString, QVariant>()).toHash();

    if (data.count())
    {
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            if (it.value().userType() != QMetaType::QVariantMap)
            {
                data.clear();
                break;
            }

            map = it.value().toMap();
            for (int j = 0; j < 2; ++j)
            {
                if (map[keys[j]].userType() != values[j])
                {
                    data.clear();
                    spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                    goto typeFailed;
                }
            }

            if (Model::Utils::verifyIP(map["ip"].toString().toUtf8().toStdString()))
            {
                data.clear();
                spdlog::warn("{}:{} Type failed", __FILE__, __LINE__);
                goto typeFailed;
            }
        } // end for (auto i = 0; i < m_data.length(); ++i)
    } // end if (data.count())

    _state.data = data;
    _state.dataIsDirty = true;
    updateData();

typeFailed:

    if (!Controller::Global::guiGlobal.isNotMobile())
    {
        isInit.store(true, std::memory_order_relaxed);
        emit InitDone();
        return;
    }

    uint_fast8_t count(0);
    while (!Controller::Global::guiGlobal.isLocalAvailable())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
        ++count;
        if (count > 5)
        {
            break;
        }
    }

    isInit.store(true, std::memory_order_relaxed);
    emit InitDone();
}

void RemoteClient::connectToServerImpl()
{}

uint_fast8_t RemoteClient::dataInternal()
{
    if (_state.data.isEmpty())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "no data");
        return 1;
    }

    int lastIndex = LAST_INDEX;
    if (IS_NOT_VALID_INDEX(lastIndex))
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid index");
        _state.dataIsDirty = true;
        return 1;
    }

    m_dataName = _state.dataCache.at(lastIndex);
    m_data = _state.data[m_dataName].toMap();
    return 0;
}

} // namespace GUI

} // end namespace Controller

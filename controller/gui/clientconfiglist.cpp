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

#include "spdlog/spdlog.h"
#include "QVariant"
#include "QSettings"

#include "controller/global/init.hpp"
#include "global.hpp"
#include "clientconfiglist.hpp"

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
    bool dataIsNotDirty = false;
    QList<QString> dataCache;
} _STATE;

static _STATE _state;

ClientConfigList::ClientConfigList(QObject *parent) :
    QObject(parent)
{
    updateData();
}

ClientConfigList::~ClientConfigList()
{
    if (!_state.data.isEmpty())
    {
        QSettings s;
        s.setValue("Server List", _state.data);
    }
}

uint_fast8_t ClientConfigList::data(QString &outName,
                                    QMap<QString, QVariant> &outMap)
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
        _state.dataIsNotDirty = false;
        return 1;
    }

    outName = _state.dataCache.at(lastIndex);
    outMap = _state.data[outName].toMap();
    return 0;
}

QJSValue ClientConfigList::data()
{

    if (_state.dataCache.length() <= 0)
    {
        return Controller::Global::guiGlobal.engine()->newArray(0);
    }

    int startIndex = _state.lastPage * COUNT_PER_PAGE;
    int dataCount = _state.dataCache.length() - startIndex;
    if (dataCount <= 0)
    {
        _state.dataIsNotDirty = false;
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid index");
        return Controller::Global::guiGlobal.engine()->newArray(0);
    }

    if (dataCount > 5) dataCount = 5;

    QJSValue ret = Controller::Global::guiGlobal.engine()->newArray(dataCount);
    for (int i = 0, j = startIndex; i < dataCount; ++i, ++j)
    {
        ret.setProperty(i, _state.dataCache.at(j));
    }

    return ret;
}

int ClientConfigList::dataPages() const
{
    return _state.totalPages;
}

int ClientConfigList::pageIndex() const
{
    return _state.lastPage;
}

bool ClientConfigList::isNoData() const
{
    return _state.dataCache.empty();
}

void ClientConfigList::updateData()
{
    if (_state.dataIsNotDirty) return;

    _state.dataCache.clear();
    _state.dataIsNotDirty = true;
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

void ClientConfigList::updateDataQml()
{
    return updateData();
}

void ClientConfigList::setData(const QString &name,
                               const QMap<QString, QVariant> &data)
{
    _state.data[name] = data;
    _state.dataIsNotDirty = false;
}

void ClientConfigList::setData(const QHash<QString, QVariant> &in)
{
    _state.data = in;
    _state.dataIsNotDirty = false;
}

void ClientConfigList::setLastPage(int pageIndex, int index)
{
    int dataIndex = DATA_INDEX(pageIndex, index);
    if (IS_NOT_VALID_INDEX(dataIndex))
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid index");
        return;
    }

    _state.lastPage = pageIndex;
    _state.lastDataIndex = index;
}

void ClientConfigList::deleteData(const QString &name)
{
    auto it = _state.data.find(name);
    if (it == _state.data.end())
    {
        spdlog::error("{}:{} {}", __FILE__, __LINE__, "invalid name");
        return;
    }

    _state.data.erase(it);
    _state.dataIsNotDirty = false;
}

} // namespace GUI

} // end namespace Controller

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

#include "stqqueuelist.hpp"

uint8_t STQQueueList::createQueue(const std::string &name)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    std::string tmpString;
    if (!m_queueList.empty())
    {
        if (m_queueList.find(name) != m_queueList.end()) return 1;
    }

    auto toBeInserted = std::make_shared<STQQueue>();
    m_queueList[name] = toBeInserted;
    return 0;
}

uint8_t STQQueueList::renameQueue(const std::string &oldName,
                                  const std::string &newName)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    std::string tmpString;
    if (!m_queueList.empty())
    {
        auto it = m_queueList.find(oldName);
        if (it != m_queueList.end())
        {
            auto queue = it->second;
            m_queueList.erase(it);
            m_queueList[newName] = queue;
            return 0;
        }
    }

    return 1;
}

uint8_t STQQueueList::deleteQueue(const std::string &name)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    std::string tmpString;
    if (!m_queueList.empty())
    {
        auto it = m_queueList.find(name);
        if (it != m_queueList.end())
        {
            m_queueList.erase(it);
            return 0;
        }
    }

    return 1;
}

std::vector<std::string> STQQueueList::listQueue()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    std::vector<std::string> ret;
    if (m_queueList.size() > 0)
    {
        ret.reserve(m_queueList.size());
    }

    std::string tmpString;
    if (!m_queueList.empty())
    {
        for (auto it = m_queueList.begin();
             it != m_queueList.end();
             ++it)
        {
            tmpString = it->first;
            ret.push_back(tmpString);
        }
    }

    return ret;
}

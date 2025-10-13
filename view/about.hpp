/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
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

#ifndef _VIEW_ABOUT_HPP_
#define _VIEW_ABOUT_HPP_

#include "QObject"

#include "config.h"

namespace View
{

class About : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString version READ getVersion CONSTANT FINAL)

    Q_PROPERTY(QString branch READ getBranch CONSTANT FINAL)

    Q_PROPERTY(QString commit READ getCommit CONSTANT FINAL)

public:

    explicit About(QObject *parent = nullptr);

    QString getVersion() const;

    QString getBranch() const;

    QString getCommit() const;

    Q_INVOKABLE void aboutQt();

private:

    QString m_version = FF_VERSION;

    QString m_branch = FF_BRANCH;

    QString m_commit = FF_COMMIT;
};

} // namespace View

#endif // _VIEW_ABOUT_HPP_

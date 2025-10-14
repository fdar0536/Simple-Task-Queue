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

#include "QDebug"
#include "QIcon"
#include "QQuickStyle"
#include "QQuickWindow"

#include "controller/global/global.hpp"
#include "controller/gui/global.hpp"

// gui
#include "view/about.hpp"
#include "view/main.hpp"
#include "view/settings.hpp"

#include "fmt/core.h"

#include "main.hpp"

namespace Controller
{

namespace GUI
{

static QObject *globalProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return Controller::GUI::Global::instance();
}

// public member functions
Main::Main()
{}

Main::~Main()
{
    Controller::GUI::Global::destroy();
}

u8 Main::init(QApplication &app)
{
    if (Controller::Global::isAdmin())
    {
        fmt::println("{}:{} Refuse to run as admin.", __FILE__, __LINE__);
        return 1;
    }

    QQuickStyle::setStyle("Material");

    app.setWindowIcon(QIcon("://original-icon.jpg"));
    auto global = Controller::GUI::Global::instance();
    if (!global)
    {
        fmt::println("{}:{} Fail to initialize global", __FILE__, __LINE__);
        return 1;
    }

    registQmlTypes();

    auto engine = global->engine();
    engine->load(QUrl("qrc:/qt/qml/FF/view/main.qml"));
    if (engine->rootObjects().isEmpty())
    {
        qCritical("Fail to load qml");
        return 1;
    }

    QQuickWindow *window = qobject_cast<QQuickWindow*>
        (engine->rootObjects().constFirst());
    if (window)
    {
        window->setIcon(app.windowIcon());
    }

    m_app = &app;
    return 0;
}

i32 Main::run()
{
    return m_app->exec();
}

// private member functions
void Main::registQmlTypes()
{
    qmlRegisterSingletonType<Controller::GUI::Global>
        ("ff.backend.global", 1, 0, "Global", globalProvider);

    qmlRegisterType<View::Main>("ff.backend.main", 1, 0, "Main");

    qmlRegisterType<View::Settings>("ff.backend.settings", 1, 0, "Settings");
    qmlRegisterType<View::About>("ff.backend.about", 1, 0, "About");
}

} // end namespace GUI

} // end namespace Controller

/*
 * Simple Task Queue
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

#ifndef _CONTROLLER_GLOBAL_INIT_HPP_
#define _CONTROLLER_GLOBAL_INIT_HPP_

#include <inttypes.h>

#include "config.hpp"
#include "defines.hpp"

#ifndef STQ_MOBILE
#include "controller/grpcserver/server.hpp"
#include "model/dao/sqlitequeuelist.hpp"
#endif

namespace Controller
{

namespace GUI
{

class Global;

}

namespace Global
{

extern Config config;

#ifndef STQ_MOBILE
extern GRPCServer::Server server;

extern std::shared_ptr<Model::DAO::IQueueList> sqliteQueueList;
#endif

#ifdef STQ_GUI
extern Controller::GUI::Global guiGlobal;
#endif

uint_fast8_t init(int argc, char **argv);

void fin();

#ifndef STQ_MOBILE
uint_fast8_t initSQLiteQueueList();
#endif

} // end namespace Global

} // end namespace Controller

#endif // _CONTROLLER_GLOBAL_INIT_HPP_

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

#include "spdlog/spdlog.h"
#include "grpcpp/server_builder.h"

#include "server.hpp"

#include "controller/global/init.hpp"

namespace Controller
{

namespace GRPCServer
{

Server::Server()
{
#ifdef STQ_GUI
    m_isRunning.store(false, std::memory_order_relaxed);
#endif
}

Server::~Server()
{
    stop();
}

uint_fast8_t Server::start()
{
#ifdef STQ_GUI
    if (m_isRunning.load(std::memory_order_relaxed))
    {
        spdlog::warn("{}:{} Server is running", __FILE__, __LINE__);
        return 0;
    }
#endif

    try
    {
        grpc::ServerBuilder builder;
        std::string listenAddr = Global::config.listenIP() + ":" +
                                 std::to_string(Global::config.listenPort());
        int actualPort(0);
        builder.AddListeningPort(listenAddr,
                                 grpc::InsecureServerCredentials(),
                                 &actualPort);

        builder.RegisterService(&m_accessImpl);
#ifdef STQ_GUI
        m_isRunning.store(true, std::memory_order_relaxed);
#endif
        m_server = builder.BuildAndStart();
        spdlog::info("{}:{} Server is listening on {}", __FILE__, __LINE__,
                     listenAddr);

        auto serveFn = [this]()
        {
            this->m_server->Wait();
        };

        m_thread = std::jthread(serveFn);
#ifndef STQ_GUI
        m_future = m_exitRequested.get_future();
        m_future.wait();
        m_server->Shutdown();
        m_thread = std::jthread();
        m_server = nullptr;
#endif
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to start server", __FILE__, __LINE__);
        m_server = nullptr;
#ifdef STQ_GUI
        m_isRunning.store(false, std::memory_order_relaxed);
#endif
        return 1;
    }

    return 0;
}

void Server::stop()
{
#ifdef STQ_GUI
    try
    {
        m_server->Shutdown();
        m_thread = std::jthread();
        m_server = nullptr;
        m_isRunning.store(false, std::memory_order_relaxed);
    }
    catch (...)
    {
        spdlog::error("{}:{} Fail to stop server", __FILE__, __LINE__);
    }
#else
    m_exitRequested.set_value();
#endif
}

} // end namespace GRPCServer

} // end namespace Controller

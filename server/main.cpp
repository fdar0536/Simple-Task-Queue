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

#include <iostream>

#include <cinttypes>
#include <csignal>
#include <cstring>

#ifdef _WIN32
#ifdef __MINGW32__
#include "getopt.h"
#else
#include "win32-code/getopt.h"
#endif // __MINGW32__
#else
#include "unistd.h"
#include "getopt.h"
#endif // _WIN32

// grpc
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "common.hpp"
#include "global.hpp"

// controllers
#include "controller/accessimpl.hpp"
#include "controller/consoleimpl.hpp"
#include "controller/doneimpl.hpp"
#include "controller/pendingimpl.hpp"
#include "controller/queueimpl.hpp"

static void printHelp(char **);

static void runServer(bool);

static bool isAdmin();

static void sighandler(int signum);

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType);
#endif

int main(int argc, char **argv)
{
    if (isAdmin())
    {
#ifdef _WIN32
        std::cerr << "Refuse to run as administrator." << std::endl;
#else
        std::cerr << "Refuse to run as super user." << std::endl;
#endif
        return 1;
    }

    struct option opts[] =
    {
        {"help",   no_argument,       NULL, 'h'},
        {"debug",  no_argument,       NULL, 'd'},
        {"config", required_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };

    int c;
    char configFile[4096] = {};
    bool debug(false);
    while ((c = getopt_long(argc, argv, "hdc:", opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'h':
        {
            printHelp(argv);
            return 0;
        }
        case 'd':
        {
            debug = true;
            break;
        }
        case 'c':
        {
            size_t len = strlen(optarg);
            if (len > 4095)
            {
                printHelp(argv);
                return 1;
            }

            memcpy(configFile, optarg, len);
            configFile[len] = '\0';
            break;
        }
        default:
        {
            printHelp(argv);
            return 0;
        }
        } // end switch(c)
    }

    if (Global::init(configFile))
    {
        return 1;
    }

    signal(SIGABRT, sighandler);
    signal(SIGFPE, sighandler);
    signal(SIGILL, sighandler);
    signal(SIGINT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);

#ifdef _WIN32
    SetConsoleCtrlHandler(eventHandler, TRUE);
#endif

    runServer(debug);
    return 0;
}

static bool isAdmin()
{
#ifdef _WIN32
    PSID sid;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&auth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &sid))
    {
        return true;
    }

    BOOL res;
    if (!CheckTokenMembership(nullptr, sid, &res))
    {
        return true;
    }

    FreeSid(sid);
    return res;
#else
    return (geteuid() == 0);
#endif
}

static void runServer(bool debug)
{
    if (debug)
    {
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    }

    grpc::ServerBuilder builder;
    int actualPort(0);
    builder.AddListeningPort(Global::ip + ":" + Global::port,
                             grpc::InsecureServerCredentials(),
                             &actualPort);

    AccessImpl accessImpl;
    builder.RegisterService(&accessImpl);

    QueueImpl queueImpl;
    builder.RegisterService(&queueImpl);

    PendingImpl pendingImpl;
    builder.RegisterService(&pendingImpl);

    DoneImpl doneImpl;
    builder.RegisterService(&doneImpl);

    ConsoleImpl consoleImpl;
    builder.RegisterService(&consoleImpl);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server is listening on ";
    std::cout << Global::ip << ":" << actualPort;
    if (debug)
    {
        std::cout << " with debug mode." << std::endl;
    }
    else
    {
        std::cout << std::endl;
    }

    auto serveFn = [&]()
    {
        server->Wait();
    };

    std::thread serving_thread(serveFn);
    auto f = Global::exit_requested.get_future();
    f.wait();
    server->Shutdown();
    serving_thread.join();
}

static void printHelp(char **argv)
{
    std::cout << argv[0] << " usage:" << std::endl;
    std::cout << "-h, --help: Print this message and exit." << std::endl;
    std::cout << "-d, --debug: Start server with debug mode." << std::endl;
    std::cout << "-c, --config <config file>: Path to config file." << std::endl;
}

static void sighandler(int signum)
{
    UNUSED(signum);
    Global::logger.write(Logger::Info, "Good Bye!");
    Global::exit_requested.set_value();
}

#ifdef _WIN32
static BOOL eventHandler(DWORD dwCtrlType)
{
    UNUSED(dwCtrlType);
    sighandler(0);
    return TRUE;
}
#endif

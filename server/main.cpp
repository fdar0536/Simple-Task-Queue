#include <iostream>

#include <cinttypes>
#include <cstring>

#ifdef _WIN32
#ifdef __MINGW32__
#include "getopt.h"
#else
#include "win32-code/getopt.h"
#endif // __MINGW32__
#else
#include "getopt.h"
#endif // _WIN32

// grpc
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "global.hpp"

// controllers
#include "controller/queueimpl.hpp"

void printHelp(char **argv)
{
    std::cout << argv[0] << " usage:" << std::endl;
    std::cout << "-h, --help: Print this message and exit." << std::endl;
    std::cout << "-d, --debug: Start server with debug mode." << std::endl;
    std::cout << "-c, --config <config file>: Path to config file." << std::endl;
}

void runServer(bool debug)
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

    QueueImpl queueImpl;
    builder.RegisterService(&queueImpl);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on ";
    std::cout << Global::ip << ":" << actualPort;
    if (debug)
    {
        std::cout << " with debug mode." << std::endl;
    }
    else
    {
        std::cout << std::endl;
    }

    server->Wait();
}

int main(int argc, char **argv)
{
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

    runServer(debug);
    return 0;
}

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

#include "global.hpp"

void printHelp(char **argv)
{
    std::cout << argv[0] << " usage:" << std::endl;
    std::cout << "-h, --help: Print this message and exit." << std::endl;
    std::cout << "-c, --config <config file>: Path to config file." << std::endl;
}

int main(int argc, char **argv)
{
    struct option opts[] =
    {
        {"help",   no_argument,       NULL, 'h'},
        {"config", required_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };

    int c;
    char configFile[4096] = {};
    while ((c = getopt_long(argc, argv, "hc:", opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'h':
        {
            printHelp(argv);
            return 0;
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

    return 0;
}

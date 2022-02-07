#pragma once

#include <inttypes.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef __MINGW32__
#define STQ_EXPORT __attribute__((dllexport))
#else
#define STQ_EXPORT __declspec(dllexport)
#endif // __MINGW32__
#else
#define STQ_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum STQ_Priority
{
    STQ_High,
    STQ_AboveNormal,
    STQ_Normal,
    STQ_BelowNowmal,
    STQ_Low
} STQ_Priority;

typedef struct STQ_FactoryPack
{
    char *dst;
    char *workDir;
    char *programName;
    STQ_Priority priority;
    int argc;
    char **argv;
    uint8_t saveOutput;
    int exitcode;
    char *postHandler;
    uint8_t haveToDelete;
} STQ_FactoryPack;

STQ_EXPORT void STQ_PostHandler(STQ_FactoryPack *);

#ifdef __cplusplus
}
#endif

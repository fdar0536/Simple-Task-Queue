#pragma once

#include <string>
#include <vector>

typedef struct TaskDetails
{
    std::string workDir;
    std::string programName;
    std::vector<std::string> args;
    uint32_t exitCode;

    // for copy struct
    TaskDetails& operator=(const TaskDetails &);
} TaskDetails;

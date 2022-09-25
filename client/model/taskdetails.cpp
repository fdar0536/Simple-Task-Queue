#include "taskdetails.hpp"

TaskDetails& TaskDetails::operator=(const TaskDetails &other)
{
    if (this == &other) return *this;

    workDir = other.workDir;
    programName = other.programName;
    args = other.args;
    exitCode = other.exitCode;
    priority = other.priority;

    return *this;
}

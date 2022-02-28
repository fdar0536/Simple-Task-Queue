 #include "stqtask.hpp"

STQTask& STQTask::operator=(const STQTask &other)
{
    if (this == &other) return *this;

    id = other.id;
    execName = other.execName;
    workDir = other.workDir;
    args = other.args;
    postScript = other.postScript;
    priority = other.priority;
    exitCode = other.exitCode;
    isSuccess = other.isSuccess;

    return *this;
}

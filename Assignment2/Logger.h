#include <string>


enum Severity
{
    DEBUG=1,
    WARNING,
    ERROR,
    CRITICAL
};

int InitializeLog();
void SetLogLevel(Severity severity);
void Log(Severity severity, const char* filename, const char* func_name, int line_num, const char* log_msg);
void ExitLog();
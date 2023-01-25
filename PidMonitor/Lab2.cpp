#include <iostream>
#include <string>
#include "pidUtil.h"

int main(void)
{
    vector<int> pidList = {};
    std::cout << "Reading all pids in the system..." << std::endl;
    std::cout << GetErrorMsg(GetAllPids(pidList)) << std::endl;

    std::cout << "Getting names of all pids..." << std::endl;
    for (int pid : pidList)
    {
        std::string name;
        if (GetNameByPid(pid, name) == Err_OK)
        {
            std::cout << "PID: " << std::to_string(pid) << ", Name: " << name << std::endl;
        }        
    }

    std::string name;
    std::cout << "Finding name of pid 1..." << std::endl;
    std::cout <<  GetErrorMsg(GetNameByPid(1, name)) << std::endl;
    std::cout << "The name of pid 1: " << name <<std::endl;
    
    int pid;
    std::cout << "Finding pid of Lab2..." << std::endl;
    std::cout << GetErrorMsg(GetPidByName("Lab2", pid)) << std::endl;
    std::cout << "Lab2 pid: " << pid << std::endl;

    std::cout << "Finding pid of Lab22..." << std::endl;
    std::cout << GetErrorMsg(GetPidByName("Lab22", pid)) << std::endl;

    return 0;
}
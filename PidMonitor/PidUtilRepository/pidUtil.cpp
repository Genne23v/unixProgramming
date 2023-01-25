#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fstream>
#include "pidUtil.h"

ErrStatus GetAllPids(vector<int> &pidList) 
{
    DIR *dir;
    struct dirent *ent;
    char folder[1000];

    dir = opendir("/proc");
    if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (ent->d_type == DT_DIR)
            {
                int pid = atoi(ent->d_name);
                if (pid > 0)
                {
                    pidList.push_back(pid);
                }
            }
        }
        closedir (dir);
        return Err_OK;
    }
    else
    {
        return Err_DirOpen;
    }
}

string GetErrorMsg(ErrStatus index)
{
    std::string errMsg = "";

    switch(index)
    {
        case Err_OK:
            errMsg = "Operation was successful!";
            break; 
        case Err_DirOpen:
            errMsg = "Could not open the directory";
            break;
        case Err_NoPid:
            errMsg = "Could find the pid";
            break;
        case Err_NoName:
            errMsg = "Could find process name";
            break;
    }
    return errMsg;
}

ErrStatus GetNameByPid(int pid, string &name)
{
    DIR *dir;
    struct dirent *ent;
    std::string path;

    dir = opendir("/proc");
    if (dir != NULL)
    {
        while ((ent = readdir (dir)) != NULL)
        {
            if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) 
            {
                if (stoi(ent->d_name) == pid)
                {
                    path = std::string("/proc/") + ent->d_name + "/status";
                    std::ifstream file(path);
                    std::string line;
                    
                    while (std::getline(file, line, '\n')) 
                    {
                        if (line.substr(0, 4) == "Name") 
                        {
                            name = line.substr(6); 
                        }
                    }
                }
            }
        }
        closedir (dir);
        return Err_OK;
    }
    else
    {
        return Err_DirOpen;
    }
}

ErrStatus GetPidByName(string name, int &pid)
{
    DIR *dir;
    struct dirent *ent;
    std::string path;
    
    dir = opendir("/proc");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
                path = std::string("/proc/") + ent->d_name + "/status";
                std::ifstream file(path);
                std::string line;
                
                while (std::getline(file, line, '\n')) {
                    if (line.substr(0, 4) == "Name") 
                    {
                        if (line.substr(6) == name)
                        {
                            pid = stoi(ent->d_name);
                            file.close();
                            return Err_OK;
                        } 
                    } 
                }
                file.close();
            }
        }
        closedir(dir);
        return Err_NoName;
    } else {
        return Err_DirOpen;
    }

}
    

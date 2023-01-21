#include <iostream>
#include <fstream>
#include <dirent.h>

int main() {
    DIR *dir;
    struct dirent *ent;
    std::string path;
    std::string name, pid, vmSize, vmRss;
    int i=0;

    dir = opendir("/proc");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR && isdigit(ent->d_name[0])) {
                path = std::string("/proc/") + ent->d_name + "/status";
                std::ifstream file(path);
                std::string line;
                
                while (std::getline(file, line, '\n')) {
                    if (line.substr(0, 4) == "Name") {
                        name = line.substr(6); 
                    } else if (line.substr(0, 3) == "Pid") {
						pid = line.substr(4);
                    } else if (line.substr(0, 7) == "VmSize:") {
                        vmSize = line.substr(8); 
                    } else if (line.substr(0, 6) == "VmRSS:") {
						vmRss = line.substr(7);
					}
                }
                
                if (stoi(vmRss) >= 10000) {
					std::cout << ++i << ". Name: " << name << ", Pid: " << pid << ", VmSize: " << vmSize << ", VmRSS: " << vmRss << std::endl;
					std::cout << "----------------------------------------------------------------------------------------------" <<std::endl;
				}
                file.close();
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Could not open directory" << std::endl;
    }
    return 0;
}

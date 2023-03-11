#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

using namespace std;

char socket_path[] = "/tmp/lab6";
const int BUF_LEN=512;

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    char buf[256];
    int fd,rc;
    bool isRunning = true;

    memset(&addr, 0, sizeof(addr));
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        cout << "client socket error: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        cout << "client connect error: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    while(isRunning)
    {
        memset(buf, 0, sizeof(buf));
        rc = recv(fd, buf, sizeof(buf), 0);
        if (rc < 0) 
        {
            cerr << "Error receving data" << strerror(errno) << endl;
        }
        else if (rc == 0)
        {
            cout << "Server socket has closed the connection" << endl;
        }
        else 
        {
            if (strncmp("Pid", buf, 3) == 0)
            {
                cout << "A request for the client's pid has been received" << endl;
                int pid = getpid();
                char msg[BUF_LEN] = "This client has pid ";
                string pid_string = to_string(pid);
                const char *pid_cstr = pid_string.c_str();
                strcat(msg, pid_cstr);
                strncpy(buf, msg, strlen(msg));
                send(fd, buf, sizeof(buf), 0);
            }

            if (strncmp("Sleep", buf, 5) == 0)
            {
                cout << "This client is going to sleep for 5 seconds" << endl;
                sleep(5);
                strncpy(buf, "Done", 4);
                send(fd, buf, strlen(buf), 0);
            }

            if (strncmp("Quit", buf, 5) == 0)
            {
                cout << "This client is quitting" << endl;
                isRunning = false;
            }    
        }
    }

    close(fd);
    return 0;
}
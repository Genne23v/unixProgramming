#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "Logger.h"

#define BUF_LEN 1024
#define PORT 1153

using namespace std;

pthread_mutex_t lock_x;
pthread_t tid;
sockaddr_in addr;
char buf[BUF_LEN];
const char IP_ADDR[] = "127.0.0.1";
bool is_running;
int log_level, fd, len;

void *recv_func(void *arg);

int InitializeLog()
{
    // Create a non-blocking socket for UDP 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        cerr << "Error: Failed to create socket" << endl;
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        cerr << "Error getting socket flags: " << strerror(errno) << endl;
        close(fd);
        return 1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "Error setting socket to non-blocking mode: " << strerror(errno) << endl;
        close(fd);
        return 1;
    }

    // Set the address and port of the server.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    addr.sin_port = htons(PORT);
    
    cout<<"Connecting to server..."<<endl;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    is_running = true;
    
    const char connectToServer[] = "Logger connecting to the server\n";
    sendto(fd, connectToServer, sizeof(connectToServer), 0, (struct sockaddr*)&addr, sizeof(addr));

    int ret = pthread_create(&tid, NULL, recv_func, &fd);
    if (ret != 0)
    {
        cout<<"Cannot create thread"<<endl;
        cout<<strerror(errno)<<endl;
        return -1;
    }
    
    return 0;
}

void SetLogLevel(Severity severity)
{
    log_level = severity;     
    cout << "Log Level set to " << log_level << endl;;
}

void Log(Severity severity, const char* filename, const char* func_name, int line_num, const char* log_msg)
{
    if (log_level <= severity)
    {
        time_t now = time(0);
        char *dt = ctime(&now);
        memset(buf, 0, BUF_LEN);
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[severity-1], filename, func_name, line_num, log_msg)+1;
        buf[len-1]='\0';
    
        // Send the log to the server
        sendto(fd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
        cout << "Log sent to the server" << endl;
    }
}

void ExitLog()
{
    cout << "Loggeris being terminated" << endl;
    is_running = false;
    pthread_join(tid, NULL);
    pthread_mutex_destroy(&lock_x);
    close(fd);
}

void *recv_func(void *arg)
{
    fd = *(int *)arg;
    socklen_t addr_len = sizeof(addr);
    while(is_running)
    {
        memset(buf, 0, BUF_LEN);
        len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr*)&addr, &addr_len);
        cout << "Waiting for a message from server..." << endl;
        // Sleep a sec if nothing is received
        if(len<0) sleep(1);
        else {
            // Only command from the server
            const char setLogLevel[] = "Set Log Level";
            if (strncmp(buf, setLogLevel, strlen(setLogLevel)) == 0)
            {
                int level;
                sscanf(buf, "Set Log Level=%d", &level);

                Severity severity;
                switch (level) {
                    case 1:
                        severity = DEBUG;
                        break;
                    case 2:
                        severity = WARNING;
                        break;
                    case 3:
                        severity = ERROR;
                        break;
                    case 4:
                        severity = CRITICAL;
                        break;
                    default:
                        cerr << "Invalid log level: " << level << endl;
                }
                SetLogLevel(severity);
            }
        }
    }
    pthread_exit(NULL);
}

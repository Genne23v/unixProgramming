#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#define BUF_LEN 1024
#define PORT 1153

using namespace std;

pthread_mutex_t lock_x;
pthread_t tid;
struct sockaddr_in serv_addr, cli_addr;
bool is_running;
const char IP_ADDR[] = "127.0.0.1";
int file_fd;

void *recv_func(void *arg);
void set_sock(int fd);

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            // The server will exit its user menu.
            break;
    }
}


int main()
{
    char buf[BUF_LEN];
    int len;

    pthread_mutex_init(&lock_x, NULL);

    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    // Create a non-blocking socket for UDP communications
    int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1) {
        cerr << "Error: Failed to create socket" << endl;
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(serv_sock, F_GETFL, 0);
    fcntl(serv_sock, F_SETFL, flags | O_NONBLOCK);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr);
    if (ret == 0)
    {
        cout << "No such address" << endl;
        cout << strerror(errno) << endl;
        close(serv_sock);
        return -1;
    }
    serv_addr.sin_port = htons(PORT);

    // Bind the socket to its IP address and to an available port.
    int bind_ret = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bind_ret == -1) {
        cerr << "Error: Failed to bind socket" << endl;
        exit(EXIT_FAILURE);
    }

    is_running = true;
    cout << "Waiting on " << inet_ntoa(serv_addr.sin_addr) << endl;

    set_sock(serv_sock);
    ret = pthread_create(&tid, NULL, recv_func, &serv_sock);
    if (ret != 0)
    {
        cout<<"Cannot create thread"<<endl;
        cout<<strerror(errno)<<endl;
        return -1;
    }

    // Start a receive thread and pass the file descriptor to it.
    while(is_running)
    {
        int selection;
        cout << "Select an option to handle the log (1. Set the log level, 2. Dump the log file, 0. Shutdown): ";
        cin >> selection;

        switch(selection)
        {
            case 0:
                is_running = false;
                break;
            case 1:
                int log_level;
                cout << "Set the log level (1. DEBUG, 2. WARNING, 3. ERROR, 4. CRITICAL): ";
                cin >> log_level;

                // Send the log level to logger
                memset(buf, 0, BUF_LEN);
                len=sprintf(buf, "Set Log Level=%d", log_level)+1;
                ret = sendto(serv_sock, buf, len, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
                if (ret < 0){
                    cerr << "Error: Failed to send a message" << strerror(errno) << endl;
                }
                break;
            case 2:
                // Open the log file as read-only and dump its contents to console
                pthread_mutex_lock(&lock_x);
                ifstream logfile("./server.log", ios_base::in);
                if (logfile.is_open())
                {
                    string line;
                    while(getline(logfile, line))
                    {
                        cout << line << endl;
                    }
                    logfile.close();

                    while (getchar() != '\n') {}
                    cout << "Press any key to continue:";
                    getchar();
                }
                else
                {
                    cerr << "Error: Failed to open log file" << endl;
                }
                pthread_mutex_unlock(&lock_x);
                break;
        }
    }

    pthread_join(tid, NULL);
    pthread_mutex_destroy(&lock_x);
    close(serv_sock);

    return 0;
}

void *recv_func(void *arg)
{
    int *int_arg = static_cast<int*>(arg);
    int fd = *int_arg;
    char buf[BUF_LEN] = {};
    socklen_t cli_addr_len = sizeof(cli_addr);

    while(is_running) 
    {
        ssize_t num_bytes = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (num_bytes == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                perror("recvfrom");
                break;
            }
        } else if (num_bytes == 0) {
            // Sleep for a sec if nothing is received
            sleep(1);
            break;
        } else {
            pthread_mutex_lock(&lock_x);
            file_fd = open("./server.log", O_CREAT | O_WRONLY | O_APPEND, 0666);
            if (file_fd < 0) {
                cerr << "Error: Failed to open server.log" << strerror(errno) << endl;
                exit(1);
            }

            if (write(file_fd, buf, num_bytes) < 0){
                cerr << "Error: Failed to write data to file: " << strerror(errno) << endl;
                exit(1);
            }
            pthread_mutex_unlock(&lock_x);
        }
    }
    pthread_exit(NULL);
}

void set_sock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        cerr << "Error getting socket flags: " << strerror(errno) << endl;
        close(fd);
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "Error setting socket to non-blocking mode: " << strerror(errno) << endl;
        close(fd);
        return;
    }
 
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}
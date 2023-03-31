#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <queue>
#include <pthread.h>
#include <vector>

#define BUF_SIZE 1024
#define NUM_THREADS 3

using namespace std;

queue<string> msg_queue;
pthread_mutex_t lock_x;
bool is_running;

void *recv_func(void *arg);
void set_cli_sock(int fd);

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}

int main(int argc, char *argv[])
{
    is_running=false;
    pthread_t tid[NUM_THREADS];
    
    pthread_mutex_init(&lock_x, NULL);

    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        exit(EXIT_FAILURE);
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        cerr << "Error: Failed to create socket" << endl;
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(serv_sock, F_GETFL, 0);
    fcntl(serv_sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = htons(atoi(argv[1]));

    int bind_ret = bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bind_ret == -1) {
        cerr << "Error: Failed to bind socket" << endl;
        exit(EXIT_FAILURE);
    }

    int listen_ret = listen(serv_sock, SOMAXCONN);
    if (listen_ret == -1) {
        cerr << "Error: Failed to listen on socket" << endl;
        exit(EXIT_FAILURE);
    }

    is_running = true;
    cout << "Server is listening on port " << argv[1] << endl;
    int threadNum = 0;
    vector<int> cli_fds;

    while(is_running)
    {
        struct sockaddr_in cli_addr;
        socklen_t cli_addr_len = sizeof(cli_addr);
        // Each client is connected to unique fd
        int cli_sock = accept(serv_sock, (struct sockaddr*)&cli_addr, &cli_addr_len);
        if (cli_sock == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                continue;
            }
            else 
            {
                cerr << "Error: Failed to accept connection" << endl;
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            cli_fds.push_back(cli_sock);
            // Once client is connected, set timeout and create a thread to communicate through fd
            set_cli_sock(cli_sock);

            int ret = pthread_create(&tid[threadNum], NULL, recv_func, &cli_sock);
            if (ret != 0)
            {
                cout<<"Cannot create thread"<<endl;
                cout<<strerror(errno)<<endl;
                return -1;
            }
            threadNum++;

            cout << "Accepted a new connection from " << inet_ntoa(cli_addr.sin_addr) << endl;
        }
    }
    // Send Quit to all clients
    for (int i=0; i<NUM_THREADS; i++)
    {
        write(cli_fds[i], "Quit", 4);
        close(cli_fds[i]);
    }

    for (int i=0; i<threadNum; i++)
    {
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&lock_x);
    close(serv_sock);

    return 0;
}

void *recv_func(void *arg)
{
    int *int_arg = static_cast<int*>(arg);
    int fd = *int_arg;
    char buf[BUF_SIZE] = {};

    while(is_running) 
    {
        ssize_t num_bytes = recv(fd, buf, sizeof(buf), 0);
        if (num_bytes == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                perror("recv");
                break;
            }
        } else if (num_bytes == 0) {
            break;
        } else {
            pthread_mutex_lock(&lock_x);
            msg_queue.push(string(buf));
            pthread_mutex_unlock(&lock_x);
        }
        // If queue is not empty, print the message in the queue and pop it out
        if (msg_queue.size() > 0)
        {
            pthread_mutex_lock(&lock_x);
            cout<<"msg from client: "<<msg_queue.front()<<endl;
            msg_queue.pop();
            pthread_mutex_unlock(&lock_x);
        }
        else 
        {
            sleep(1);
        }
    }
    pthread_exit(NULL);
}

void set_cli_sock(int cli_fd) {
    int flags = fcntl(cli_fd, F_GETFL, 0);
    if (flags == -1) {
        cerr << "Error getting socket flags: " << strerror(errno) << endl;
        close(cli_fd);
        return;
    }
    if (fcntl(cli_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        cerr << "Error setting socket to non-blocking mode: " << strerror(errno) << endl;
        close(cli_fd);
        return;
    }
    // Set timeout to 5 seconds
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(cli_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}


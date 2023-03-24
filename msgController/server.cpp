#include <errno.h> 
#include <iostream> 
#include <queue> 
#include <signal.h> 
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <unistd.h>
#include "client.h"

using namespace std;

key_t key;
int msgid;
bool is_running;
queue<Message> message;
pthread_mutex_t lock_x;

void *recv_func(void *arg);
void *send_func(void *arg);

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}

int main()
{
    int ret;
    pthread_t tid_r, tid_w;

    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    key = ftok("serverclient", 65);
    msgid = msgget(key, 0666 | IPC_CREAT); 

    pthread_mutex_init(&lock_x, NULL);
    is_running=true;

    ret = pthread_create(&tid_r, NULL, recv_func, NULL);
    if(ret!=0) 
    {
        is_running = false;
        cout<<strerror(errno)<<endl;
        return -1;
    }

    ret = pthread_create(&tid_w, NULL, send_func, NULL);
    if(ret!=0) 
    {
        is_running = false;
        cout<<strerror(errno)<<endl;
        return -1;
    }

    pthread_join(tid_r, NULL);
    pthread_join(tid_w, NULL);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return 1;
    }

    return 0;
}

void *recv_func(void *arg)
{
    while(is_running) 
    {
        Message msg;
        msgrcv(msgid, &msg, sizeof(msg), 4, 0);
        cout<<"msg received from client "<<msg.msgBuf.source<<": "<<msg.msgBuf.buf<<endl;
        if(strncmp(msg.msgBuf.buf, "Quit", 4)==0) 
        {
            is_running=false;
        }
        else 
        {
            pthread_mutex_lock(&lock_x);
            message.push(msg);
            pthread_mutex_unlock(&lock_x);
        }
    }
    pthread_exit(NULL);
}

void *send_func(void *arg)
{
    while(is_running)
    {
        if (message.size() > 0)
        {
            pthread_mutex_lock(&lock_x);
            Message sendMsg = message.front();
            message.pop();
            pthread_mutex_unlock(&lock_x);
            sendMsg.mtype = 0;
            msgsnd(msgid, &sendMsg, sizeof(sendMsg), 0);
            cout<<"msg sent to client "<<sendMsg.msgBuf.dest<<": "<<sendMsg.msgBuf.buf<<endl;
        }
    }
    pthread_exit(NULL);
}
// client.h - Header file for shared memory
//
// 23-Jul-20  M. Watler         Created.
//
#ifndef CLIENT_H
#define CLIENT_H

const char SEMNAME[]="FileSem";
const char MEMNAME[]="MemDispatch";
const mode_t SEM_PERMS=(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
const int BUF_LEN=1024;
const int NUM_MESSAGES=30;

struct Memory {
    int            packet_no;
    unsigned short srcClientNo;
    unsigned short destClientNo;
    char           message[BUF_LEN];
};

void *recv_func(void *arg);
#endif//CLIENT_H

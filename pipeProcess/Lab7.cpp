#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>

const int LEN=32;

int main (int argc, char **argv)
{
    char argument1[LEN];
    char argument2[LEN];
    strcpy(argument1, argv[1]);
    strcpy(argument2, argv[2]);
    char arg1[3][LEN];
    char arg2[3][LEN];
    int len1=0, len2=0;
    int pfd[2];
    pid_t pid1, pid2, pid;

    char *token = strtok(argument1, " ");
    while(token != NULL)
    {
        strcpy(arg1[len1], token);
        token = strtok(NULL, " ");
        ++len1;
    }

    token = strtok(argument2, " ");
    while(token != NULL)
    {
        strcpy(arg2[len2], token);
        token = strtok(NULL, " ");
        ++len2;
    }
    
    if (pipe(pfd) == -1)
    {
        perror("pipe");
    }
    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        execlp(arg1[0], arg1[0], arg1[1], NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else 
    {
        close(pfd[1]);
        dup2(pfd[0], STDIN_FILENO);
        execlp(arg2[0], arg2[0], arg2[1], arg2[2], NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    return 0;
}
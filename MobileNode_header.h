#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>
#include <sys/fcntl.h>

#define PIPE "TASK_PIPE"
#define BUF_PIPE 512

typedef struct task_struct {
    int num_requests;
    float interval_time;
    int num_commands;
    float max_time;
    // int id; não é necessário, variavel global incrementada
} Task;

Task *tasks;
int id = 0;
int named_pipe;


int MobileNode(int argc, char *argv[]);
int sendRequest(int num_commands, int max_time);
int error(char *title, char *message);
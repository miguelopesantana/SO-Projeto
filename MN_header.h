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

typedef struct task_struct {
    int num_requests;
    float interval_time;
    int num_commands;
    float max_time;
    int id;
} Task;

Task *tasks;
int id = 0;

int error(char *title, char *message);
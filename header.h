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
#include "functions.c"

#define N 10
#define MIN_LEN 64
#define MAX_LEN 1024

int readFile();

int error(char* title, char* message);

void addLog(char* mensagem);

typedef struct config_struct {
    int num_slots; //Número de slots na fila interna do Task Manager
    int max_wtime; //Tempo máximo para que o processo Monitor eleve o nível de performance dos Edge Servers
    int num_servers; //Número de Edge Servers
    Server* servers; //array of servers
} Config;

//struct with server's information
typedef struct server_struct {
    char* name[MIN_LEN];
    int vCPU1;
    int vCPU2;
} Server;

typedef struct memory {
    int num_slots; //Número de slots na fila interna do Task Manager
    int max_wtime; //Tempo máximo para que o processo Monitor eleve o nível de performance dos Edge Servers
    int num_servers; //Número de Edge Servers
    Server* servers[]; //array of servers

    pthread_mutex_t mutex;

} Data;

struct task {
    int num_pedidos;
    int time_betw_tasks;
    int num_tasks;
    int max_time;
}


Task *tasks;
Data *shared_data;
sem_t * mutex_log;
sem_t * mutex_write;
int shmid;
pthread_t my_thread[N];
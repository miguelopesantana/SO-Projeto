#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>
#include <sys/fcntl.h>

#define SIZE 100
#define N 10
#define MAX_LENGTH 256
#define NUM_SERVERS 100

typedef struct memory {
    int num_slots; //Número de slots na fila interna do Task Manager
    int max_wtime; //Tempo máximo para que o processo Monitor eleve o nível de performance dos Edge Servers
    int num_servers; //Número de Edge Servers
    EdgeServer servers[NUM_SERVERS]; //array of servers

    pthread_mutex_t mutex;

} Data;

struct tarefa {
    int num_pedidos;
    int time_betw_tasks;
    int num_tasks;
    int max_time;
}

//struct with server's information
typedef struct server {
    char name[256];
    int vcpu1;
    int vcpu2;

} EdgeServer;

Task *tasks;
Data *shared_data;
sem_t * mutex_log;
sem_t * mutex_write;
int shmid;
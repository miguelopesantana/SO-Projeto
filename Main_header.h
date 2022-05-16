#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/fcntl.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include <signal.h>

#include <sys/msg.h>
#include <errno.h>

#define MIN_LEN 64
#define MAX_LEN 1024
#define PIPE_NAME "TASK_PIPE"
#define BUF_PIPE 512

//struct with server's information
typedef struct{
    //no semaphore needed for access
    int edge_server_num;
    char* name;
    int vCPU1;
    int vCPU2;
    int pipe[2];

    //need semaphore to access
    int performance_mode;
    int maintenance_sig;
    int available_vCPUs[2];
    int executed_tasks;
    int maintenance_tasks;
    
} Edge_Server;

//edge server struct
typedef struct{
    int cpu;
    char task_buf[512];
}cpu_args;

//EDGE SERVER VARIABLES
int num_edge_server;
pthread_t cpu_threads[2];

typedef struct{
    long msg_type;
    int msg_content;
} msg;

typedef struct{
    //configuration variables
    int num_slots; //Número de slots na fila interna do Task Manager
    int max_wtime; //Tempo máximo para que o processo Monitor eleve o nível de performance dos Edge Servers
    int num_servers; //Número de Edge Servers

    int non_executed_tasks;

    //processes variables
    pid_t child_pids[3]; //task manager, monitor and maintenance manager

    //semaphores
    sem_t *log_write_mutex;
    sem_t *shm_write;
    pthread_mutex_t shm_edge_servers;
    sem_t *check_performance_mode;

    pthread_condattr_t attr_cond;
    pthread_cond_t edge_server_sig;

    pthread_cond_t edge_server_move;

    //task manager queue
    int node_number;
    pthread_mutexattr_t attr_mutex;
    pthread_mutex_t sem_tm_queue;
    pthread_cond_t new_task_cond;


    //maintenance manager message queue
    int msgqid;

    //general edge servers performance mode
    int performance_mode;

    int total_response_time;

    //variables used when system is exiting
    pthread_cond_t end_system_sig;

} Data;

//shared memory variables
int shm_id;
Data* Shared_Memory;
Edge_Server* edge_servers;

typedef struct{
    int num_pedidos;
    int time_betw_tasks;
    int num_tasks;
    int max_time;
    int ID;
} Task;

sem_t * mutex_log;
sem_t * mutex_write;
int shmid;

//task manager header
typedef struct{
    int id_node;
    int priority;
    int num_instructions;
    int timeout;
    struct tm arrive_time;
    struct Node* next_node;
}Node;

typedef struct{
    Node * first_node;
    int node_number;
}linked_list;

int node_id;
linked_list *msg_stack;

pid_t *edge_servers_proc;
int named_pipe_file;
pthread_t tm_threads[2];


/* System Manager functions  */
int readFile(char* file_name);
int SystemManager(char* file);
int initProc(void (*function)());

/* Edge Server functions */
int EdgeServer(int edge_server_number);

/* Task Manager functions */

//Process
void TaskManager();
void end_sig_tm();

void thread_cleanup(void* arg);

/* Monitor functions */
pthread_t monitor_end;

void Monitor();
void *MonitorWork();
void thread_cleanup_monitor(void* arg);

/* Maintenance Manager functions */
void MaintenanceManager();
//Maintenance and Dispatcher


/* main functions */
void clean();
int closeAll();

int error(char* title, char* message);


void addLog(char* mensagem);


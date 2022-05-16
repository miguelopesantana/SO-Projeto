#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/fcntl.h>
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
    //configuration variables
    int num_slots; //Número de slots na fila interna do Task Manager
    int max_wtime; //Tempo máximo para que o processo Monitor eleve o nível de performance dos Edge Servers
    int num_servers; //Número de Edge Servers
    int non_executed_tasks; //número de tarefas não executadas

    //processes variables
    pid_t procIDs[3]; //task manager, monitor and maintenance manager

    //semaphores
    //sem_t *log_write_mutex;
    sem_t * mutex_log;
    sem_t *shm_write;
    pthread_mutex_t shm_servers;
    sem_t *evaluate_performance_mode;

    pthread_condattr_t attr_cond;
    pthread_cond_t edge_server_sig;

    //task manager queue
    int task_number;
    pthread_mutexattr_t attr_mutex;
    pthread_mutex_t t_queue_sem;
    pthread_cond_t new_task_cond;

    //general edge servers performance mode
    int performance_mode;


    //variable used when system is exiting
    pthread_cond_t end_system_signal;

} Data;

//shared memory variables
int shm_id;
Data* Shared_Memory;
Edge_Server* edge_servers;


//task manager header
typedef struct task{
    int taskID;
    int priority;
    int num_instructions;
    int timeout;
    struct tm arrive_time;
    struct task* next_task;
}Ltask;

typedef struct{
    Ltask * first_task;
    int task_number;
}task_list;

int task_counter;
task_list *msg_stack;

pid_t *edge_servers_proc;
int named_pipe_file;
pthread_t tm_threads[2];


// System Manager functions  
int readFile(char* file_name);
int SystemManager(char* file);
int initProc(void (*function)());

/* Edge Server functions */
int EdgeServer(int edge_server_number);

/* Task Manager functions */
void TaskManager();
void endSystemSignal();
void* endSystem();

/* Monitor functions */
pthread_t monitor_end;

void Monitor();
void controlMonitor();

/* Maintenance Manager functions */
void MaintenanceManager();

/* main functions */
void clean();
int closeAll();

int error(char* title, char* message);
void addLog(char* mensagem);


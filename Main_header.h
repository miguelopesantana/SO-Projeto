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
#include "SystemManager_functions.c"
#include "SystemManager.c"
#include "TaskManager.c"
#include "EdgeServer.c"
#include "Monitor.c"
#include "MaintenanceManager.c"

#define MIN_LEN 64
#define MAX_LEN 1024
#define PIPE_NAME "TASK_PIPE"
#define BUF_PIPE 512

//struct with server's information
typedef struct server_struct {
    //no semaphore needed for access
    int edge_server_num;
    char* name[MIN_LEN];
    int cpu1_cap;
    int cpu2_cap;
    int pipe[2];

    //need semaphore to access
    int performance_mode;
    int in_maintenance;
    int available_vCPUs[2];
    int executed_tasks;
    int maintenance_tasks;
    
} Edge_Server;

//edge server struct
typedef struct{
    int cpu;
    char tas_buf[512];
}args_cpu;

//EDGE SERVER VARIABLES
int global_edge_server_number;
pthread_t cpu_threads[2];

typedef struct{
    long msg_type;
    int msg_content;
} msg;

typedef struct shm_struct {
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
Edge_Server* edge_server_lists;

typedef struct task_struct {
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
    int num_instructions:
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


pid_t *edge_servers_processes;
int named_pipe_file;
pthread_t tm_threads[2];
int readFile();

void initSim();

int error(char* title, char* message);

void addLog(char* mensagem);

/* Edge Server functions */
int EdgeServer(int edge_server_number);
void* vCPU(void* args);
void* endMonitor();
void doMaintenance(pid_t es_pid);


/* System Manager functions  */

int init(char* file_name)
int SystemManager(char* file);
int check_regex(char *text, char *regex);

/* Task Manager functions */

//linked list
void insert(linked_list** lista, int priority, int num_instructions, int timeout);
int remove(linked_list** lista,int id_node);
void checkPriorities(linked_list** lista);
void getNextTask(linked_list **lista, Node** next_task);
void clean(linked_list** lista);

//Process
int TaskManager();
void end_sig_tm();
void* scheduler();
void* dispatcher();
void* endMonitorTM();

void check_cpus(Node *next_task, int **flag, int **pipe_to_send);
int try_to_send(Node *next_task);
int time_since_arrive(Node *task);

void thread_cleanup(void* arg);

/* Monitor functions */
pthread_t monitor_end;

int Monitor();
void *MonitorWork();
void thread_cleanup_monitor(void* arg);

/* Maintenance Manager functions */
int MaintenanceManager();
pid_t* list_pids;
void sigint_maintenance();

//Maintenance and Dispatcher
pthread_cond_t edge_server_move;

/* main functions */
void cleanup();
void sigint();
void sigtstp();

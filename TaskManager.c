#include "Main_header.h"

// Função Task Manager

void TaskManager(){
    
    //open unnamed pipes
    int flags;
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        pipe(edge_servers[i].pipe);

    }

    edge_servers_proc = malloc(sizeof(pid_t) * (Shared_Memory->num_servers));

    //Start Edge Servers
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if((edge_servers_proc[i] = fork()) == 0){
            EdgeServer(i);
            exit(0);
        }
    }

    //open named pipe for reading
    if((named_pipe_file = open(PIPE_NAME, O_RDWR)) < 0){
        addLog("Cannot open pipe");
        endSystemSignal();
        exit(1);
    }
    addLog("Task pipe opened");

    //criar message queue
    task_counter = 0;
    msg_stack = (task_list*)malloc(sizeof(task_list));
    Shared_Memory->task_number = 0;
    msg_stack->first_task = NULL;


    //Dispatcher Thread
    pthread_create(&tm_threads[1], NULL, dispatcher, 0);

    //Schedular Thread
    pthread_create(&tm_threads[0], NULL, scheduler, 0);

    //condicao variavel à espera que o system acabe
    endSystem();

}

//thread dispatcher
void *dispatcher(){
    
}

//thread sheduler
void *scheduler(){

}

void endSystemSignal(){
    char buffer[512];
    Ltask* aux = msg_stack->first_task;

    addLog("Cleaning up Task Manager");

    // fechar as threads scheduler e dispatcher
    pthread_cancel(tm_threads[0]);
    pthread_cancel(tm_threads[1]);


    // Escrever no log as mensagens que restam na fila do sheduler
    sem_wait(Shared_Memory->shm_write);
    while(aux != NULL){
        snprintf(buffer,sizeof(buffer),"Task %d undone",aux->taskID);
        addLog(buffer);
        aux = aux->next_task;
        Shared_Memory->non_executed_tasks++;
    }
    sem_post(Shared_Memory->shm_write);

    // limpa a message queue
    free(msg_stack);

    // fecha named pipe
    unlink(PIPE_NAME);
    close(named_pipe_file);

    // Espera que os processos Edge Server terminem
    for (int i = 0; i < Shared_Memory->num_servers; i++){
        wait(NULL);
    }


    // CLOSE UNNAMED PIPES
    for(int i = 0; i<Shared_Memory->num_servers;i++){
        close(edge_servers[i].pipe[0]);
        close(edge_servers[i].pipe[1]);
    }

    free(edge_servers_proc);

    addLog("Task Manager cleanup complete");
    exit(0);
}

void* endSystem(){


    pthread_cond_wait(&Shared_Memory->end_system_signal, &Shared_Memory->t_queue_sem);

    endSystemSignal();

    pthread_mutex_unlock(&Shared_Memory->t_queue_sem);

    exit(0);

}

void insertTask(task_list **list, int priority, int num_instructions, int timeout){

    time_t now;

    // Create new node
    Ltask *new_task =(Ltask *)malloc(sizeof(Ltask));
    new_task->taskID = task_counter++;
    new_task->priority = priority;
    new_task->num_instructions = num_instructions;
    new_task->timeout = timeout;
    new_task->next_task = NULL;

    time(&now);
    localtime_r(&now, &new_task->arrive_time);

    Ltask *aux = (*list)->first_task;

    if (aux == NULL){
        (*list)->first_task = new_task;
    }else{
        // Search for insertion place
        while (aux->next_task != NULL){
            aux = aux->next_task;
        }

        // Insert
        aux->next_task = new_task;
    }

    Shared_Memory->task_number++;
}

int removeTask(task_list **list, int task_id){

    Ltask *aux = (*list)->first_task;

    // Verificar se é primeiro nó
    if (aux->taskID == task_id){

        (*list)->first_task = aux->next_task;
        free(aux);
        Shared_Memory->task_number--;
        return 0;
    }else{ // Se não é o primeiro nó

        while ((aux->next_task != NULL) && (aux->next_task->taskID != task_id)){
            aux = aux->next_task;
        }

        // Chegámos ao ultimo node e o id não corresponde
        if (aux->next_task == NULL){
            return 1;
        }else{
            
            Ltask *delete = aux->next_task;
            aux->next_task = aux->next_task;
            free(delete);
            Shared_Memory->task_number--;

            return 0;
        }
    }
}
#include "SM_header.h"

int main(int argc, char *argv[]){
    
    //read config file
    readFile();

    //Block CTRL+c and CTRL+z during initialization
    //Todo block all signals, after init resume only CTRL+c and CTRL+z
    
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    if(argc == 2)
        init(argv[1]);
    else{
        fprintf(stderr, "Wrong command format\n");
        return 1;
    }

    //Redirect CTRL+c and CTRL+z
    signal(SIGTSTP, sigtstp);
    signal(SIGINT,sigint);

    return 0;
}


void clean(){
    
    //sinal para avisar os processos que é para terminar
    pthread_cond_broadcast(&Shared_Memory->end_system_sig);

    //Espera aqui, só pode fechar a shared memory quando todos os outros processos fecharem
    for(int i = 0; i<3; i++){
        wait(NULL);
    }

    //print estatísticas
    sigtstp();

    //maintenance manager message queue
    msgctl(Shared_Memory->msgqid, IPC_RMID, NULL);

    //sempahores
    sem_unlink("SHM_WRITE");
    sem_unlink("SHM_CHECK_PFM");
    sem_close(Shared_Memory->shm_write);
    sem_close(Shared_Memory->check_performance_mode);

    pthread_cond_destroy(&Shared_Memory->edge_server_sig);
    pthread_cond_destroy(&Shared_Memory->end_system_sig);
    pthread_cond_destroy(&Shared_Memory->new_task_cond);
    pthread_condattr_destroy(&Shared_Memory->attr_cond);
    
    pthread_mutex_destroy(&Shared_Memory->shm_edge_servers);
    pthread_mutex_destroy(&Shared_Memory->sem_tm_queue);
    pthread_mutexattr_destroy(&Shared_Memory->attr_mutex);

}

//CLOSING SYSTEM
void sigint(){

    addLog("CLEANING UP RESOURCES");
    cleanup();
    addLog("CLEANUP COMPLETE! CLOSING SYTEM");~

    //close log sem
    sem_unlink("LOG_WRITE_MUTEX");
    sem_close(Shared_Memory->log_write_mutex);

    //shared memory
    shmdt(Shared_Memory);
    shmctl(shm_id, IPC_RMID, NULL);

    //close after, to keep the log file updated

    exit(0);
}

//SYSTEM STATS
void sigtstp(){
    char buffer[BUFSIZ];
    int total_tasks = 0;

    pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

    for(int i = 0; i < Shared_Memory->num_servers; i++){
        memset(buffer, 0, BUFSIZ);
        snprintf(buffer, BUFSIZ, "Completed tasks at Edge Server %d: %d", i, edge_server_lists[i].executed_tasks);
        addLog(buffer);

        total_tasks += edge_server_lists[i].executed_tasks;
        
    }

    pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

    sem_wait(Shared_Memory->shm_write);

    memset(buffer, 0, BUFSIZ);
    if(total_tasks != 0){
        snprintf(buffer, BUFSIZ, "Mean of response time between tasks: %d", (int)Shared_Memory->total_response_time/total_tasks);
    }else{
        snprintf(buffer, BUFSIZ, "Mean of response time between tasks: 0");
    }
    addLog(buffer);

    memset(buffer, 0, BUFSIZ);
    snprintf(buffer,BUFSIZ,"Total number of completed tasks: %d", total_tasks);
    addLog(buffer);

    memset(buffer, 0, BUFSIZ);
    snprintf(buffer,BUFSIZ,"Total number of non-completed tasks: %d", Shared_Memory->non_executed_tasks);
    addLog(buffer);

    sem_post(Shared_Memory->shm_write);

}

// Função Task Manager

int TaskManager(){
    
    //open unnamed pipes
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        pipe(edge_server_lists[i].pipe);
    }

    edge_servers_processes = malloc(sizeof(pid_t) * (Shared_Memory->num_servers));

    //Start Edge Servers
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if((edge_servers_processes[i] = fork()) == 0){
            EdgeServer(i);
            exit(0);
        }
    }

    //open named pipe for reading
    if((name_pipe_file = open(PIPE_NAME, O_RDWR)) < 0){
        addLog("Cannot open pipe");
        end_sig_tm();
        exit(1);
    }
    addLog("Task pipe opened");

    //create message queue
    node_id = 0;
    msg_stack = (linked_list *)malloc(sizeof(linked_list));
    Shared_Memory->node_number = 0
    msg_stack->first_node = NULL;

    //handle en signal
    //signal(SIGINT, end_sig_tm);

    //monitor
    pthread_t monitor;
    pthread_create(&monitor, NULL, endMonitorTM,0);

    //Dispatcher Thread
    pthread_create(&tm_threads[1], NULL, dispatcher, 0);

    //Schedular Thread
    pthread_create(&tm_threads[0], NULL, scheduler, 0);

    //condiçao variável à espera que o system acabe
    pthread_join(monitor, NULL);

    return 0;
}


void *scheduler(){

    char pipe_buffer[BIF_PIPE];
    int number_read;
    int id_task;
    int num_instructions;
    int timeout_priority;
    char *tok;
    char *rest;
    memset(pipe_buffer, 0, BUF_PIPE);

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(named_pipe_file, &read_set);

    /*
        for (int i = 0; i < 3; i++)
        {

            num_instructions = i;
            timeout_priority = i;

            // Reevaluate priorities and insert into message list
            pthread_mutex_lock(&rd_wr_list);
            check_priorities(&fila_mensagens);

            insert_list(&fila_mensagens, timeout_priority, num_instructions, timeout_priority);
            insert_list(&fila_mensagens, timeout_priority*2, num_instructions*2, timeout_priority*2);
            insert_list(&fila_mensagens, timeout_priority*3, num_instructions*3, timeout_priority*3);
            insert_list(&fila_mensagens, timeout_priority*5, num_instructions*5, timeout_priority*5);


            Node *aux = fila_mensagens->first_node;
            while (aux != NULL)
            {
                printf("No: %d,%d,%d\n", aux->id_node, aux->num_instructions, aux->priority);
                aux = aux->next_node;
            }

            //if(fila_mensagens->node_number == 1){
                // Avisar o dispatcher que já há mensangens na fila
                pthread_cond_signal(&new_task_cond);
            //}


            pthread_mutex_unlock(&rd_wr_list);



            printf("antes sleep\n");
            sleep(5);
            printf("depois\n");
        }
        */

    //espera por mensagens no TASK_PIPE
    if(select(name_pipe_file + 1, &read_Set, NULL, NULL, NULL) > 0){
        if(FD_ISSEST(name_pipe_file, &read_set)){


            number_read = read(name_pipe_file, buffer_pipe, BUF_PIPE);
            buffer_pipe[number_read] = "\0";

            if(number_read > 0){
                
                //Todo check if it's a QUIT or STATS command before continuing

                //split arguments
                tok = atrtok_r(buffer_pipe, ";", &rest);
                id_task = atoi(tok);
                id_task++;

                tok = strtok_r(NULL,";",&resto);
                num_instructions = atoi(tok);

                tok = strtok_r(NULL,";",&resto);
                timeout_priority = atoi(tok);

                pthread_mutex_lock(&Shared_Memory->sem_tm_queue);

                //check if message queue is full
                if(Shared_Memory->node_number == Shared_Memory->num_slots){
                    //log
                    addLog("TASK MANAGER QUEUE FULL, MESSAGE DISCARDED");

                    sem_wait(Shared_Memory->shm_write);
                    Shared_Memory->non_executed_tasks++;
                    sem_post(Shared_memory->shm_write);

                }else{
                    //reeavaluate priorities and insert into message list
                    check_priorities(&msg_stack);
                    insert_list(&msg_stack, timeout_priority, num_instructions, timeout_priority);

                    //avisa o monitor para verificar o load da fila
                    //avisar o dispatcher que há mensagens na fila

                    pthread_cond_broadcast(&Shared_Memory->new_task_cond);

                }
                pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);
            }else{
                printf("recebeu 0\n");
            }
        }
    }
    pthread_exit(NULL);

}

void *dispatcher(){

    pthread_cleanup_push(thread_cleanup_handler, NULL);

    Node *next_task = (Node*)malloc(sizeof(Node));

    while(1){
        pthread_mutex_lock(&Shared_Memory->sem_tm_queue);

        if(Shared_Memory->node_number == 0){
            pthread_cond_wait(&Sahred_Memory->new_task_cond, &Shared_Memory->sem_tm_queue);
        }

        //ir buscar a task com maior prioridade
        get_next_task(&msg_stack, &next_task);

        pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);

        //try to send the task to some edge server . if there are no vCPUs available, waits for a signal saying that some vCPU just ended a task
        pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
        while(try_to_send(new_task) == 1){

            //nenhum edge server disponivel, esperar por o sinal de algum deles e verificar novamente
            addLog("DISPATCHER: ALL EDGE SERVERS BUSY, WAITING FOR AVAILABLE vCPUs");
            pthread_cond_wait(&Shared_Memory->edge_server_sig, &Shared-Memory->shm_edge_servers);
            addLog("DISPATCHER: 1 CPU BECAME AVAILABLE\n");
        }
        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
        usleep(2000);
    }
    free(next_task);
    pthread_cleanup_pop(0);
    pthread_exit(NULL);
}

int try_to_send(Node *next_task){
    int flag = 0, pipe_to_end = -1;
    int *flag_ptr = &flag; //check if there is any available vCPU
    int *pipe_to_send = &pipe_to_send;

    //check if vCPUs are available
    check_cpus(next_task, &flag_ptr, &pipe_to_send);

    char task_str[512];
    memset(task_str, 0, sizeof(task_str));

    if(flag == 1 && pipe_to_send == -1){
        //descartada

        //mean response time / non executed tasks
        sem_wait(Shared_Memory->shm_write);
        Shared_Memory->total_response_time += time_since_arrive(next_task);
        Shared_Memory->non_executed_tasks++;
        sem_post(Shared_Memory->shm_write);

        //discard task
        snprintf(task_str, 512, "TASK %d DISCARDED AT DISPATCHER: NO TIME TO COMPLETE", next_task->id_node);
        addLog(task_str);
        return 0;
    }else if(flag == 1){
         
        //send to server
        snprintf(task_str, 512,"%d;%d", next_task->id_node, next_task->num_instructions);
        write(edge_server_list[pipe_to_send].pipe[1], &task_str, sizeof(task_Str));

        //log
        snprintf(task_str, sizeof(task_str), "TASK %d SELECTED %d FOR EXECUTION ON %s", next_task->id_node, next_task->priority, edge_server_lists[pipe_to_send].name);
        addLog(task_str);
        memeset(task_str, 0, sizeof(task_str));

        //mean response time
        sem_wait(Shared_Memory->shm_write);
        Shared_Memory->total_responde_time += time_since_arrive(next_task);
        sem_post(Shared_Memory->shm_write);

        return 0;
    }else{
        return 1;
    }
}

//Check for available vCPUs in the Edge Servers

void check_vcpus(Node *next_task, int **flag, int **pipe_to_send){
    time_t now;
    struct tm check_time;

    time(&now);
    localtime_r(&now, &check_time);
    int tempo_decorrido = (abs(check_time.tm_min - next_task->arrive_time.tm_min)%60)*60 + abs(check_time.tm_sec - next_task->arrive_time.tm_sec)%60;
    int tempo_restante = next_task->timeout - tempo_decorrido;

    //pthread_mutex_lock(&SMV->shm_edge_servers);
    //Check if there is any available CPU and, if so, check if it has capacity to run the task in time
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if(edge_server_lists[i].available_vCPUs[0] == 1){ //CPU1 available on Edge server i
            if(next_task->num_instructions/edge_server_lists[i].cpu1_cap <= tempo_restante){ //CPU1 has capacity to run the task in time
                *(*pipe_to_send) = i;
                *(*flag) = 1;
                break;
            }
            *(*flag) = 1;
        }
        if(edge_server_lists[i].available_vCPUs[1] == 1){ //CPU2 available on Edge server i
            if(next_task->num_instructions/edge_server_lists[i].cpu2_cap <= tempo_restante){ //CPU2 has capacity to run the task in time
                *(*pipe_to_send) = i;
                *(*flag) = 1;
                break;
            }
            *(*flag) = 1;
        }
    }
    //pthread_mutex_unlock(&SMV->shm_edge_servers);
}

void end_sig_tm(){
    char buffer[512];
    Node * aux = msg_stack->first_node;

    addLog("CLEANING UP TASK MANAGER");

    //Close Threads
    //pthread_mutex_lock(&SMV->shm_edge_servers);
    pthread_cancel(tm_threads[0]);
    pthread_cancel(tm_threads[1]);
    //pthread_mutex_unlock(&SMV->shm_edge_servers);

    //Escrever no log as mensagens que resta na fila do scheduler + named pipe
    for(int i = 0; i < Shared_Memory->node_number; i++){
        snprintf(buffer,sizeof(buffer), "TASK %d LEFT UNDONE", aux->id_node);
        addLog(buffer);
        aux = aux->next_node;
    }

    //clean message queue
    free(msg_stack);

    //Close Named Pipe
    unlink(PIPE_NAME);
    close(named_pipe_file);

    //wait for Edge Server Processes to finish
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        wait(NULL);
    }

    //Close unnamed pipes
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        close(edge_server_list[i].pipe[0]);
        close(edge_server_list[i].pipe[1]);
    }

    free(edge_server_processes);
    addLog("TASK MANAGER CLEANUP COMPLETE");
    exit(0);

}

void insert(linked_list * list, int priority, int num_instructions, int timeout){
    
    time_t now;

    //create new Node
    Node *new_node = (Node *) malloc(sizeof(Node));
    new_node->id_node = node_id++;
    new_node->priority = priority;
    new_node->num_instructions = num_instructions;
    new_node->timeout = timeout;
    new_node->next_node = NULL;

    time(&now);
    localtime_r(&now, &new_node->arrive_time);

    Node* node_aux = (*list).first_node;

    if(aux_ndoe == NULL){
        (*list).first_node = new_node;
    }else{
        while(node_aux->next_node != NULL){
            node_aux = node_aux->next_node;
        }
        //insert
        node_aux->next_node = new_node;

    }
    Shared_Memory->node_number++;
}

int remove(linked_list ** list, int id_node){
    Node *node_aux = (*list)->first_node;

    //verificar se é primeiro nó
    if(node_aux->id_node == id_node){
        (*list)->first_node = node_aux->next_node;
        free(node_aux);
        Shared_Memory->node_number--;
        return 0;
    }else{ //if it´s not first node
        while(node_aux->next_node != NULL) && (node_aux->next_node->id_node != id_node){
            node_aux = node_aux->next_node;
        }
        //chegámos ao último node e o id não corresponde
        if(node_aux->next_node == NULL){
            return 1;
        }else{ //if it´s the last node
            Node *node_to_delete = node_aux->next_node;
            node_aux->next_node = node_aux->next_node->next_node;
            free(node_to_delete);
            Shared_Memory->node_number--;
            return 0;
        }
    }
}

void check_priorities(linked_list ** list){

    //time_t now;
    //struct tm check_time;
    //int elapsed_min;
    int elapsed_sec;

    Node *node_aux = (*list)->first_node;

    while(node_aux != NULL){
        // Check if task has passed timeout;
        // time(&now);
        // localtime_r(&now, &check_time);

        // elapsed_minutes = abs(check_time.tm_min - aux_node->arrive_time.tm_min) % 60;
        // elapsed_seconds = elapsed_minutes * 60 + abs(check_time.tm_sec - aux_node->arrive_time.tm_sec)%60;
        elapsed_seconds = time_since_arrive(aux_node);

        if(elapsed_seconds >= aux_node->timeout){
            //escrever para o log e remover a task da fila
            addLog("TASK %d TIMED OUT", aux_node->id_node);
            //errado
            int id_to_remove = aux_node->id_node;
            node_aux = node_aux->next_node;

            remove(&msg_stack, id_to_remove);
        }else{
            //ainda não passou o timeout, reavaliar a prioridade
            node_aux->priority = aux_node->priority - elapsed_seconds;

            node_aux = node_aux->next_node; 
        }
    }
}

void get_next_task(linked_list ** list, Node ** next_task){

    Node *node_aux = (*list)->first_node;
    int most_priority_task_id = node_aux->id_node;
    int max_priority = node_aux->priority;

    (*next_task)->id_node = node_aux->id_node;
    (*next_task)->arrive_time = node_aux->arrive_time;
    (*next_task)->next_node = node_aux->next_node;
    (*next_task)->num_instructions = node_aux->num_instructions;
    (*next_task)->priority = node_aux->priority;
    (*next_task)->timeout = node_aux->timeout;

    //get the most prioritary node
    while(node_aux != NULL){
        if(node_aux->priority > max_priority){
            most_priority_task_id = node_aux->id_node;
            max_priority = node_aux->priority;

            //update the return argument with the highest priority node
            (*next_task)->id_node = node_aux->id_node;
            (*next_task)->arrive_time = node_aux->arrive_time;
            (*next_task)->next_node = node_aux->next_node;
            (*next_task)->num_instructions = node_aux->num_instructions;
            (*next_task)->priority = node_aux->priority;
            (*next_task)->timeout = node_aux->timeout;
        }
    }
    //remove the most prioritary node from the list
    remove(list, most_priority_task_id);
}

int time_since_arrive(Node * task){
    time_t now;
    struct tm check_time;
    int elapsed_min;
    int elapsed_sec;

    time(&now);
    localtime_r(&now, &check_time);

    elapsed_min = abs(check_time.tm_min - task->arrive_time.tm_min) % 60;
    elapsed_sec = elapsed_min * 60 + abs(check_time.tm_sec - task->arrive_time.tm_sec)%60;

    return elapsed_sec;
}

void endMonitorTM(){

    pthread_cond_wait(&Shared_Memory->end_system_sig, &Shared_Memory->sem_tm_queue);

    end_sig_tm();

    pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);

    pthread_exit(NULL);
}

void thread_cleanup(void *arg){
    pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);
}

// --------------------------Função Maintenance Manager----------------------------------
int MaintenanceManager(){
    signal(SIGINT,SIG_DFL);
    pid_t *pids_list = malloc(sizeof(pid_t) * Shared_Memory->num_servers);
    int server_to_maintenance;
    msg work_msg;
    char buf[512];
    
    //receber informação do servidor dizendo que está a trabalhar
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        //ler msg da msg queue
        msgrcv(Shared_Memory->msgqid, &work_msg, sizeof(work_msg) - sizeof(long), i+1, 0);
        pids_list[i] = work_msg.msg_content;
    }

    addLog("MAINTENANCE MANAGER: ALL EDGE SERVERS READY TO WORK");

    //work
    while(1){
        //sleep
        sleep(10 + rand()%5);

        server_to_maintenance = rand()%Shared_Memory->num_servers;

        //prepare message

        work_message-msgtype = pids_list[server_to_maintenance];
        work_msg.msg_content = 0;

        //write na MQ
        snprintf(buf, sizeof(buf), "SENDING EDGE SERVER %d TO MAINTENANCE", server_to_maintenance);
        addLog(buf);
        msgsnd(Shared_Memory->msgqid, &work_msg, sizeof(work_msg) - sizeof(long), 0);

        //wait for edge server saying is ready for maintenance
        msgrcv(Shared_Memory->msgqid, &work_msg, sizeof(work_msg) - sizeof(long), pids_list[server_to_maintenance] + 50, 0);

        //do maintenance
        sleep(1 + rand()%5);

        //return to edge server saying that it can continue working normally
        work_msg.msg_content = 0;
        work_msg.msg_type = pids_list[server_to_maintenance];
        msgsnd(Shared_Memory->msgqid, &work_msg, sizeof(work_msg) - sizeof(long), 0);
    }

    free(pids_list);

    return 0;

}

// ----------------------------Função Monitor---------------------------------------
int Monitor(){
    signal (SIGINT, SIG_DFL);
    while(1){
        //recebe o sinal da thread quando é colocada uma tarefa na fila
        pthread_mutex_lock(&Shared_Memory->sem_tm_queue);

        pthread_cond_wait(&Shared_Memory->new_task_cond,&Shared_Memory->sem_tm_queue);

        //printf("check monitor %d %d %f\n", SMV->node_number,SMV->QUEUE_POS, (double)SMV->node_number/ (double)SMV->QUEUE_POS);

        if( (double)Shared_Memory->node_number/ (double)Shared_Memory->num_slots > 0.8){ //se a fila está ocupada a >80%

            write_screen_log("QUEUE ALMOST FULL: CHANGING PERFORMANCE MODE TO 2");

            sem_wait(Shared_Memory->check_performance_mode);
            Shared_Memory->ALL_PERFORMANCE_MODE = 2;

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_server_lists[i].available_vCPUs[1] = 1;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);



            sem_post(Shared_Memory->check_performance_mode);


        }

        sem_wait(Shared_Memory->check_performance_mode);

        if( (Shared_Memory->all_performance_mode == 2) && ( (double)Shared_Memory->node_number/ (double)Shared_Memory->num_slots < 0.2) ){ //caiu para 20% ocupação

            addLog("CHANGING PERFORMANCE MODE TO 1: POWER SAVING");

            Shared_Memory->all_performance_mode = 1;

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_server_lists[i].available_vCPUs[1] = 0;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
        }

        sem_post(Shared_Memory->check_performance_mode);

        pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);

    }

    return 0;

}

//----------------------------System Manager--------------------------------------

int SystemManager(char* file){

    //open file
    FILE *initial_file;
    if((initial_file = fopen(file, "r")) == NULL){
        printf("ERROR: CANNOT OPEN FILE\n");
        exit(1);
    }

    //Open Log File
    //flog = fopen("log.txt","a");

    //TODO PROTEGER CONTRA MAU INPUT DO CONFIG FILE
    //Check file with regex functions
    /*
    char *all_cfg_file = (char*) malloc(sizeof(char) * 8192);

    if (fscanf(initFile,"%s",all_cfg_file) != 1){
        printf("ERROR READING CONFIG FILE\n");
        exit(1);
    }

    if( check_regex(all_cfg_file, "^([0-9]+(\r\n|\r|\n)){3}([a-zA-Z0-9_]{3,},[0-9]+,[0-9]+(\r\n|\r|\n)?){2,}$") != 0){
        printf("INVALID CONFIG FILE FORMAT\n");
        exit(1);
    }

    free(all_cfg_file);
    rewind(initFile);
    */

    //process the config file data
    int queue_pos_temp, max_wait_temp,edge_servers_number_temp;

    fscanf(initial_file, "%d\n%d\n%d\n", &queue_pos_temp, &max_wait_temp, &edge_servers_number_temp);

    //create the shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(struct Shared_Memory_Variables) + sizeof(Edge_Server)*edge_server_number_temp, IPC_CREAT | 0700);
    if(shmid < 1){
        addLog("ERROR CREATING SHARED MEMORY!");
        exit(1);
    }

    Shared_Memory = (Shared_Memory_Variables*) shmat(shmid, NULL, 0);
    if(Shared_Memory < (Shared_Memory_Variables*) 1){
        addLog("ERROR ATTACHING SHARED MEMORY!");
        //cleanup();
        exit(1);
    }

    //Create semaphores
    sem_unlink("LOG_WRITE_MUTEX");
    Shared_Memory->log_write_mutex = sem_open("LOG_WRITE_MUTEX", O_CREAT | O_EXCL, 0700, 1);
    sem_unlink("SHM_WRITE");
    Shared_Memory->shm_write = sem_open("SHM_WRITE", O_CREAT | O_EXCL, 0700, 1);
    sem_unlink("SHM_CHECK_PFM");
    Shared_Memory->check_performance_mode = sem_open("SHM_CHECK_PFM", O_CREAT | O_EXCL, 0700, 1);

    pthread_condattr_init(&Shared_Memory->attr_cond);
    pthread_condattr_stepshared(&Shared_Memory->attr_cond, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&Shared_Memory->edge_server_sig, &Shared_Memory->attr_cond);
    pthread_cond_init(&Shared_Memory->end_system_sig, &Shared_Memory->attr_cond);

    pthread_cond_init(&Shared_Memory->new_task_cond, &Shared_Memory->attr_cond);

    pthread_muttexattr_init(&Shared_Memory->attr_mutex);
    pthread_muttexattr_stepshared(&Shared_Memory->attr_mutex, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&Shared_Memory->shm_edge_servers, &Shared_Memory->attr_mutex);
    pthread_mutex_init(&Shared_Memory->sem_tm_queue, &Shared_Memory->attr_mutex);

    addLog("SHARED MEMORY CREATED");
    addLog("SHARED MEMORY ATTACHED");

    //put edge_servers on shared memory
    //read properties for each edge server
    edge_server_lists = (Edge_Server*) (Shared_Memory + 1);

    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if(i != Shared_Memory->num_servers - 1){
            fscanf(initial_file, "%s12[^,],%d,%d\n", edge_server_lists[i].name, &edge_server_lists[i].cpu1_cap, &edge_server_lists[i].cpu2_cap);
        }else{
            fscanf(initial_file, "%s12[^,],%d,%d\n", edge_server_lists[i].name, &edge_server_lists[i].cpu1_cap, &edge_server_lists[i].cpu2_cap);
        }

    }
    sem_post(Shared_Memory->shm_write);
        /*



        */

    //Create named pipe
    if( (mkfifo(edge_server_lists[i].name, 0666) < 0) ){
        addLog("ERROR CREATING FIFO");
        //cleanup();
        exit(1);
    }
    addLog("TASK PIPE CREATED");

    //Create message queue
    if(( Shared->msqid = msgget( ftok("./TASK_PIPE", 1), IPC_CREAT | 0700)) == -1){
        addLog("ERROR CREATING MAINTENANCE MANAGER MESSAGE QUEUE");
        exit(1);
    }

    //Create Processes

    //Monitor
    if(Shared_Memory->child_pids[0] = fork() == 0){
        addLog("MONITOR PROCESS CREATED");
        Monitor();
        exit(0);
    }else if(Shared_Memory->child_pids[0] == -1){
        addLog("ERROR CREATING MONITOR PROCESS. CLOSING PROGRAM...");
        exit(0);
    }

    //Task Manager
    if(Shared_Memory->child_pids[1] = fork() == 0){
        addLog("TASK MANAGER PROCESS CREATED");
        TaskManager();
        exit(0);
    }else if(Shared_Memory->child_pids[1] == -1){
        addLog("ERROR CREATING TASK MANAGER PROCESS. CLOSING PROGRAM...");
        //cleanup
        exit(2);
    }

    //Maintenance Manager
    if(Shared_Memory->child_pids[2] = fork() == 0){
        addLog("MAINTENANCE MANAGER PROCESS CREATED");
        MaintenanceManager();
        exit(0);
    }else if(Shared_Memory->child_pids[2] == -1){
        addLog("ERROR CREATING MAINTENANCE MANAGER PROCESS. CLOSING PROGRAM...");
        //cleanup
        exit(3);
    }

    fclose(initial_file);
    return 0;
}


//-------------------------------------------------------------
int check_regex(char* regex, char* string){
    regex_t reg;

    regcomp(&reg, regex, REG_EXTENDED);

    int rt = regexec(&reg, text, 0, NULL, 0);

    regfree(&reg);

    return rt;
}






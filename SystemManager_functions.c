#include "Main_header.h"

int readFile(){

    FILE *file = fopen("config.txt", "r");
    if (file == NULL){
        printf("Error: could not open file %s\n", argv[1]);
        return 1;
    }

    char buffer[MIN_LEN];
    int num;
    char *token;

    for (i = 0; i < 2; i++){
        if (fgets(buffer, MIN_LEN, file)){
            num = atoi(buffer);
            if (num <= 0){
                switch (i){
                    case 0:
                        error("Config File", "Invalid number of queue slots");
                        break;
                    case 1:
                        error("Config File", "Invalid max waiting time");
                        break;
                    }
                fclose(file);
                exit(1);
            }else{
                switch (i){
                    case 0:
                        Config.num_slots = num;
                        break;
                    case 1:
                        Config.max_wtime = num;
                        break;
                    }
            }
        }else{
            error("Config File", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    if (fgets(buffer, MIN_LEN, file)) {
        num = atoi(buffer);
        if (num <= 2) {
            error("Config File", "Invalid number edge server (must be >=2)");
            fclose(file);
            exit(1);
        } else Configs.num_servers = num;
    } else {
        error("Config File", "Config file structure error");
        fclose(file);
        exit(1);
    }

    Configs.servers = (Server *)malloc(sizeof(Server) * Configs.num_servers);

    bool error = false;
    for (i = 0; i < Configs.num_servers; i++) {
        if (fgets(buffer, MIN_LEN, file)) {
            token = strtok(buffer, ",");
            if (strcmp(token, "") == 0) error = true;
            Configs.servers[i].name=(char*)malloc(sizeof(char*));
            strcpy(Configs.servers[i].name, token);
            for (int j = 0; j < 2; j++) {        
                token = strtok(NULL, ",");
                if(atoi(token) <= 0){
                    error = true;
                    break;
                } 
                if(j==0)Configs.servers[i].vCPU1 = atoi(token);
                if(j==1)Configs.servers[i].vCPU2 = atoi(token);                
            }
            if (error) {
                error("Config File", "Server atribute's structure error");
                fclose(file);
                exit(1);
            }            
        } else {
            error("Config File", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    fclose(file);
    return 0;
}

'''
void initSim(){

    // remove ficheiro log, caso exista
    remove("log.txt");

    // create shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Data), IPC_CREAT | 0700)) < 0){
        error("SHM creation", "Error in shmget with IPC_CREAT\n");
        exit(1);
    }

    // Attach shared memory
    if ((shared_data = (Data *)shmat(shmid, NULL, 0)) == (Data *)-1){
        perror("Shmat error.\n");
        exit(1);
    }

    create_sem();

    addLog("OFFLOAD SIMULATOR STARTING");
}


//function to create a fork and execute the function with arguments
void initProc(void (*function)(), void *arg){
    if (fork() == 0){
        function(arg);
        exit(0);
    }else{
        wait(NULL);
    }
}

'''

/*------------------------------------------ EDGE SERVER ------------------------------*/
int EdgeServer(int edge_server_number){
    
    char buffer[100];

    global_edge_server_number = edge_server_number;

    //Colocar o cpu1 disponivel ao inicio
    //não são necessários mecanismos de sincronização pois cada edge server acessa a ma zona diferente da memória
    edge_server_list[global_edge_server_number].available_vcpu[0] = 1;

    //avisar maintenance manager que está a trabalhar
    //colocar mensagem na message queue

    pid_t es_pid = getpid();
    msg rcv_msg;
    rcv_msg.msgtype = global_edge_server_number + 1;
    rcv_msg.msg_content = es_pid;
    msgsnd(Shared_Memory->msqid, &rcv_msg, sizeof(rcv_msg) - sizeof(long), 0);

    //Thread that controls the end of system
    pthread_t monitor;
    pthread_create(&monitor, NULL, endMonitor, 0);

    //Log
    snprintf(buf, sizeof(buf), "%s Ready", edge_server_lists[global_edge_server_number].name);
    addLog(buf);

    //work
    char buffer[512];
    int aval_cpu1,aval_cpu2;

    args_cpu thread_args1;
    thread_args1.cpu = 1

    args_cpu thread_args2;
    thread_args2.cpu = 2

    pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

    //debug check flags
    //set read for non-blocking mode
    // int flags = fcntl(edge_server_lists[glob_edge_server_number].pipe[0],F_GETFL,0);
    // fcntl(edge_server_lists[global_edge_server_number].pipe[0],F_SETFD, flags | O_NONBLOCK);
    //printf("flags %d %d\n",global_edge_server_number,flags);
    //
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(edge_server_list[ glob_edge_server_number].pipe[0],&read_set);
    
    struct timeval select_timeout;
    select_timeout.tv_usec= 1000;

    while(1){

        doMaintenance(es_pid);

        pthread_cond_wait(&Shared_Memory->edge_server_move,&Shared_Memory->shm_edge_servers);

        doMaintenance(es_pid);

        //read from pipe for buffer

        if( (select( edge_server_lists[ global_edge_server_number].pipe[0] + 1,&read_set,NULL,NULL,&select_timeout) > 0) ){ // check whether pipe has something to read or not (pipe is in non-blocking read mode)
            if( FD_ISSET(edge_server_lists[ global_edge_server_number].pipe[0],&read_set) ){
                read(edge_server_list[global_edge_server_number].pipe[0], buffer, 512);
                pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

                //check performance mode
                sem_wait(Shared_Memory->check_performance_mode);

                if(Shared_Memory->all_performance_mode == 1){
                    sem_post(Shared_Memory->check_performance_mode);

                    //send to vCPU1
                    //set cpu as not available
                    edge_server_list[global_edge_server_number].available_vCPUs[0] = 0;
                    pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

                    //argumento para a thread
                    strcpy(thread_args1.task_buf, buffer);
                    pthread_create(&cpu_threads[0], NULL, vCPU, (void*) &thread_args1);

                    //wait for thread to die
                    pthread_join(cpu_threads[0], NULL);
                }else if(Shared_Memory->all_performance_mode == 2){

                    sem_post(Shared_Memory->check_performance_mode);

                    //check both CPU's available state
                    aval_cpu1 = edge_server_list[edge_Server_number].available_vCPUs[0];
                    aval_cpu2 = edge_server_list[edge_Server_number].available_vCPUs[1];

                    if( (aval_cpu1 == 0) && (aval_cpu2 == 0)){ //nenhum CPU disponível

                        //só vêm tarefas para o server quando há cpus disponiveis
                        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
                        printf("EDGE SERVER %d error\n", global_edge_server_number);
                        exit(1);

                    }else if(aval_cpu2 == 1){ //cpu2 disponivel

                        //send to vCPU2
                        //set cpu as not available
                        edge_server_list[global_edge_server_number].available_vCPU[1] = 0;
                        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

                        //argumentos para a thread
                        strcpy(thread_args2.task_buf, buffer);
                        pthread_create(&cpu_threads[1], NULL, vCPU, (void *) &thread_args2);

                    }else if(aval_cpu1 == 1){ //cpu1 disponivel trocar para o 2 primeiro
                    
                        //send to vCPU1
                        //set cpu as not available
                        edge_server_list[global_edge_server_number].available_vCPU[0] = 0;
                        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

                        //argumentos para a thread
                        strcpy(thread_args1.task_buf, buffer);
                        pthread_create(&cpu_threads[0], NULL, vCPU. (void *) &thread_args1);
                        pthread_detach(cpu_threads[0]);
            

                    }

                } 

            }
        }

    }

    pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

    // end_sig();

    return 0;
}

/*------------------------ vCPU ----------------------------*/
void * vCPU(void * arg){
    args_cpu *t_args = (args_cpu*) arg;
    
    //split tasks arguments with strtok
    char *tok, *rest;
    int task_id, num_instructions;
    
    tok = strtok_r(t_args->task_bif, ";", &rest);
    task_id = atoi(tok);

    tok = strtok_r(NULL, ";", &rest);
    num_instructions = atoi(tok);

    //do work
    if(t_args->cpu == 2){
        usleep( (long long int) ( (long long int) num_instructions * 1000 / (long long int) (edge_server_lists[global_edge_server_number].cpu1_cap * 1000000) * 1000000) );

    }else{
         usleep( (long long int) ( (long long int) num_instructions * 1000 / (long long int) (edge_server_lists[global_edge_server_number].cpu2_cap * 1000000) * 1000000) );
    }

    //set cpu as available and update executed tasks
    pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

    //check performance mode
    sem_wait(Shared_Memory->check_performance_mode);
    if(t_args->cpu == 2){
        if(Shared_Memory->all_performance_mode == 2){
            edge_server_lists[global_edge_server_number].available_vCPUs[t_args->cpu - 1] = 1;
        }
    } else{
        edge_server_lists[ global_edge_server_number ].available_vCPUs[ t_args->cpu - 1] = 1;
    }
    sem_post(Shared_Memory->check_performance_mode);

    edge_server_lists[ global_edge_server_number ].executed_tasks++;
 ;

    //log
    char buf[256];
    snprintf(buf, 256, "%s,CPU%d: TASK %d COMPLETED", edge_servers_list[global_edge_server_number].name, t_args->cpu, task_id);
    addLog(buf);

    //Avisar o task manager/processo ES que há um CPU disponivel;
    pthread_cond_broadcast(&Shared_Memory->edge_server_sig);
    
    pthread_mutex_unlock(&Shared_Memory->shm_edge_servers)

    pthread_exit(NULL);
}


/*----------------------------- endMONITOR ---------------------------------*/
void * endMonitor(){
    
    //wait for system manager signal saying that we should end servers
    //resolver isto dos mutexes
 
    struct timespec ts;

    pthread_cond_wait(&Shared_Memory->end_system_sig, &Shared_Memory->shm_edge_servers);

    //check performance mode and wait for threads if necessary
    // //TODO CHECK MAINTENANCE
    // while( edge_server_lists[global_edge_server_number].in_maintenance == 1){
    //     pthread_cond_wait(&SMV->edge_server_sig,&SMV->shm_edge_servers);
    // }

    // pthread_mutex_unlock(&SMV->shm_edge_servers);


    //check performance mode and wait for threads if necessary
    sem_wait(Shared_Memory->check_performance_mode);
    if(edge_server_lists[global_edge_server_number].in_maintenance == 1){
        sem_wait(Shared_Memory->check_performance_mode);

        if(Shared_Memory->all_performance_mode == 1){
            sem_post(Shared_Memory->check_performance_mode);

            //pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            //wait for vCPU1 to end the work
            while(edge_server_list[global_edge_server_number].available_vCPUs[0] == 0){
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec +=3;

                //printf("%d %d %d\n",glob_edge_server_number,edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[0],edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[1]);
                pthread_cond_timedwait(&Shared_Memory->edge_server_sig, &Shared_Memory->shm_edge_servers, &ts);

            }
        }else if(Shared_Memory->all_performance_mode == 2){
            
            sem_post(Shared_Memory->check_performance_mode);

            //pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            //wait for CPU´s to end the work
            while(edge_server_list[global_edge_server_number].available_vCPUs[0] == 0 || edge_server_list[global_edge_server_number].available_vCPUs[1] == 0){
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 3;

                //printf("%d %d %d\n",glob_edge_server_number,edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[0],edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[1]);
                pthread_cond_timedwait(&SMV->edge_server_sig,&SMV->shm_edge_servers,&ts);
                //printf("TIMER EXPIRED ON %d, CHECKING CPUS\n",glob_edge_server_number);

            }
        }
    }

    pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
    
    pthread_mutex_unlock(&m);
    pthread_mutex_destroy(&m);

    char buf[80];
    snprintf(buf, sizeof(buf), "CLOSING EDGE SERVER NO. %d", global_edge_server_number);
    addLog(buf);
    exit(0);
}

/*------------------------------- doMAINTENANCE ------------------------------------*/
void doMaintenance(pid_t es_pid){

    char buffer[512];
    msg rcv_msg;

    //check for messages from maintenance manager
    if(msgrcv(Shared_Memory->msqid,&rcv_msg, sizeof(rcv_msg) - sizeof(long), es_pid, IPC_NOWAIT) != -1){

        //received a message
        //printf("received message on edge server %d, %ld %d\n",glob_edge_server_number,rcv_msg.msgtype,rcv_msg.msg_content);
        edge_server_lists[global_edge_server_number].in_maintenance = 1;
        pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

        //check performance mode and wait for threads
        sem_wait(Shared_Memory->check_performance_mode);
        if(Shared_Memory->all_performance_mode == 1){
            sem_post(Shared_Memory->check_performance_mode);

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            while(edge_server_lists[global_edge_server_number].availablle_vCPUs[0] == 0){
                    pthread_cond_wait(&Shared_Memory->edge_server_sig,&Shared_Memory->shm_edge_servers);
                }
            //set cpu as unavailable for maintenance
            edge_server_list[global_edge_server_number].available_vCPUs[0] = 0;
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

        }else if(Shared_Memory->all_performance_mode == 2){
            sem_post(Shared_Memory->check_performance_mode);

            //boths CPU's running
            //pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            if( edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[0] == 0 && edge_server_lists[global_edge_server_number].available_vCPUs[1] == 0){
                //thread is working, wait for it
                pthread_cond_wait(&Shared_Memory->edge_Server_sig, &Shared_Memory->shm_edge_servers);

                //Check which CPU ended
                //threads are working, wait for both
                while( (edge_server_lists[global_edge_server_number].available_vCPUs[0] == 0) || (edge_server_lists[global_edge_server_number].available_vCLPUs[1] == 0)){
                    pthread_cond_wait(&Shared_Memory->edge_server_sig,&Shared_Memory->shm_edge_servers);
                }


            }else if(edge_server_list[glob_edge_server_number].available_vCPUs[0] == 0){
                //only 1 vCPU running

                //thread is working, wait for it
                while(edge_server_list[global_edge_server_number].available_vCPUs[0] == 0){
                    pthread_cond_wait(&Shared_Memory->edge_server_sig, &Shared_Memory->shm_edge_servers);
                }

            }else if(edge_server_list[glob_edge_server_number].AVAILABLE_CPUS[1] == 0){
                //only vCPU2 working

                //Thread is running, wait for it
                while(edge_server_list[global_edge_server_number].available_vCPUs[1] == 0){
                    pthread_cond_wait(%Shared_Memory->edge_server_sig, &Shared_Memory->shm_edge_servers);
                }

            }
            //set cpus unavailable
            edge_server_lists[global_edge_server_number].available_vCPUs[0] = 0;
            edge_server_lists[global_edge_server_number].available_vCPUs[1] = 0;
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
        }
        //send msg to maintenance, saying that the ES is ready to do maintenance
        rcv_msg.msgtype = es_pid + 50;
        rcv_msg.msg_content = 0;
        msgsnd(Shared_Memory->msqid, &recv_msg, sizeof(rcv_msg) - sizeof(long), 0);

        //log
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "EDGE SERVER %d GOING TO MAINTENANCE", global_edge_server_number);
        addLog(buffer);

        //wait for maintenance manager saying maintenance is done
        msgrcv(Shared_Memory->msqid, &rev_msg, sizeof(rcv_msg) - sizeof(long), es_pid, 0);

        //check performance mode, set correct vCPU's to available
        sem_wait(Shared_Memory->check_performance_mode);
        if(Shared_Memory->all_performance_mode == 1){
            sem_post(Shared_Memory->check_performance_mode);

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

            //set vCPU as available
            edge_server_list[global_edge_server_number].available_vCPUs[0] = 1;

        
        }else if(Shared_Memory->all_performance_mode == 2){
            sem_post(Shared_Memory->check_performance_mode);

            //set both vCPU's available
            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

            edge_server_list[global_edge_server_number].available_vCPUs[0] = 1;
            edge_server_list[global_edge_server_number].available_vCPUs[1] = 1;

        }

        edge_server_lists[global_edge_server_number].maintenance_tasks++;
        edge_server_lists[global_edge_server_number].in_maintenance = 0;

        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "EDGE SERVER %d ENDED MAINTENANCE", global_edge_server_number);
        addLog(buffer);

        //New vCPU's available
        pthread_cond_broadcast(&Shared_Memory->edge_server_sig);

        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
    }else{
        pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
    }
}




int error(char *title, char *message){
    addLog("[ERROR] %s: %s\n", title, message);
    return 1;
}

void addLog(char* mensagem) {
    FILE *file = fopen("log.txt","a");

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    sem_wait(mutex_log);
    printf("%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fprintf(file, "%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    sem_post(mutex_log);
    fclose(file);
}


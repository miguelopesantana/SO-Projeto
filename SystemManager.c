#include "Main_header.h"

int main(int argc, char *argv[]){

    //Block CTRL+c and CTRL+z during initialization
    //Todo block all signals, after init resume only CTRL+c and CTRL+z
    
    // signal(SIGINT, SIG_IGN);

    if(argc == 2)
        SystemManager(argv[1]);
    else{
        error("SYSTEM MANAGER COMMAND", "Command structure error\n");
        return 1;
    }

    //Redirect CTRL+c and CTRL+z
    // signal(SIGINT,closeAll);

    return 0;
}

int readFile(char* file_name){
    
    FILE *file = fopen(file_name, "r");
    if (file == NULL){
        printf("Error: could not open file %s\n", file_name);
        return 1;
    }

    char buffer[MIN_LEN];
    int num;
    char *token;

    for (int i = 0; i < 2; i++){
        if (fgets(buffer, MIN_LEN, file)){
            num = atoi(buffer);
            if (num <= 0){
                switch (i){
                    case 0:
                        error("SYSTEM MANAGER CONFIG FILE", "Invalid number of queue slots");
                        break;
                    case 1:
                        error("SYSTEM MANAGER CONFIG FILE", "Invalid max waiting time");
                        break;
                    }
                fclose(file);
                exit(1);
            }else{
                switch (i){
                    case 0:
                        Shared_Memory -> num_slots = num;
                        break;
                    case 1:
                        Shared_Memory -> max_wtime = num;
                        break;
                    }
            }
        }else{
            error("SYSTEM MANAGER CONFIG FILE", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    if (fgets(buffer, MIN_LEN, file)) {
        num = atoi(buffer);
        if (num <= 2) {
            error("SYSTEM MANAGER CONFIG FILE", "Invalid number edge server (must be >=2)");
            fclose(file);
            exit(1);
        } else Shared_Memory -> num_servers = num;
    } else {
        error("SYSTEM MANAGER CONFIG FILE", "Config file structure error");
        fclose(file);
        exit(1);
    }

    //create the shared memory
    if((shmid = shmget(IPC_PRIVATE, sizeof(Data) + sizeof(Edge_Server)*Shared_Memory -> num_servers, IPC_CREAT | 0700)) < 1){
        addLog("ERROR CREATING SHARED MEMORY!");
        exit(1);
    }

    Shared_Memory = (Data*) shmat(shmid, NULL, 0);
    if(Shared_Memory < (Data*) 1){
        addLog("ERROR ATTACHING SHARED MEMORY!");
        exit(1);
    }

    //put edge_servers on shared memory
    //read properties for each edge server
    edge_servers = (Edge_Server*) (Shared_Memory + 1);

    bool erro = false;
    for (int i = 0; i < Shared_Memory -> num_servers; i++) {
        if (fgets(buffer, MIN_LEN, file)) {
            token = strtok(buffer, ",");
            if (strcmp(token, "") == 0) erro = true;
            edge_servers[i].name = (char*)malloc(sizeof(char*));
            edge_servers[i].name = token;
            for (int j = 0; j < 2; j++) {        
                token = strtok(NULL, ",");
                if(atoi(token) <= 0){
                    erro = true;
                    break;
                } 
                if(j==0)edge_servers[i].vCPU1 = atoi(token);
                if(j==1)edge_servers[i].vCPU2 = atoi(token);                
            }
            if (erro) {
                error("SYSTEM MANAGER CONFIG FILE", "Server atribute's structure error");
                fclose(file);
                exit(1);
            }            
        } else {
            error("SYSTEM MANAGER CONFIG FILE", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    addLog("SHARED MEMORY CREATED");

    fclose(file);
    return 0;
}

// System Manager
int SystemManager(char* file){

    // Read config file and create shared memory
    readFile(file);

    FILE *fp = fopen("log.txt","a");

    // Create semaphores
    sem_unlink("LOG_WRITE_MUTEX");
    Shared_Memory->log_write_mutex = sem_open("LOG_WRITE_MUTEX", O_CREAT | O_EXCL, 0700, 1);
    sem_unlink("SHM_WRITE");
    Shared_Memory->shm_write = sem_open("SHM_WRITE", O_CREAT | O_EXCL, 0700, 1);
    sem_unlink("SHM_CHECK_PFM");
    Shared_Memory->check_performance_mode = sem_open("SHM_CHECK_PFM", O_CREAT | O_EXCL, 0700, 1);


    // Pthreads

    pthread_mutex_init(&Shared_Memory->shm_edge_servers, &Shared_Memory->attr_mutex);

    sem_post(Shared_Memory->shm_write);

    //Create named pipe
    if( (mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0666) < 0) ){
        addLog("Error creating pipe");
        closeAll();
        exit(1);
    }
    addLog("Task pipe created");

    //Monitor
    if ((Shared_Memory->child_pids[0] = initProc(Monitor)) == 0){
        addLog("Monitor created");
    }else{
        addLog("Error creating Monitor process. Closing program...");
        closeAll();
        exit(1);
    }

    //Task Manager
    if ((Shared_Memory->child_pids[1] = initProc(TaskManager)) == 0){
        addLog("Task Manager created");
    }else{
        addLog("Error creating Monitor process. Closing program...");
        closeAll();
        exit(1);
    }

    //Maintenance Manager
    if ((Shared_Memory->child_pids[2] = initProc(MaintenanceManager)) == 0){
        addLog("Maintenance Manager created");
    }else{
        addLog("Error creating Monitor process. Closing program...");
        closeAll();
        exit(1);
    }
    
    fclose(fp);
    return 0;
}

//function to create a fork and execute the function with arguments
int initProc(void (*function)()){
    if (fork() == 0){
        function();
        exit(0);
        return 0;
    }else{
        wait(NULL);
        exit(1);
        return 1;
    }
}


void *dispatcher(){
    
}

void clean(){
    
    //sinal para avisar os processos que é para terminar
    pthread_cond_broadcast(&Shared_Memory->end_system_sig);

    //Espera aqui, só pode fechar a shared memory quando todos os outros processos fecharem
    //for(int i = 0; i<3; i++){
    //    wait(NULL);
    //}
    int wait_status;
    while((wait_status = wait(NULL)) > 0 || (wait_status == -1 && errno == EINTR));

    //sempahores
    sem_unlink("SHM_WRITE");
    sem_unlink("SHM_CHECK_PFM");
    sem_close(Shared_Memory->shm_write);
    sem_close(Shared_Memory->check_performance_mode);

    pthread_cond_destroy(&Shared_Memory->edge_server_sig);
    pthread_cond_destroy(&Shared_Memory->end_system_sig);
    //pthread_cond_destroy(&Shared_Memory->new_task_cond);
    pthread_cond_destroy(&Shared_Memory->edge_server_move);
    pthread_condattr_destroy(&Shared_Memory->attr_cond);
    
    pthread_mutex_destroy(&Shared_Memory->shm_edge_servers);
    pthread_mutex_destroy(&Shared_Memory->sem_tm_queue);
    pthread_mutexattr_destroy(&Shared_Memory->attr_mutex);

}

//CLOSING SYSTEM
int closeAll(){

    addLog("Closing System Manager and cleaning everything");
    clean();

    //close log sem
    sem_unlink("LOG_WRITE_MUTEX");
    sem_close(Shared_Memory->log_write_mutex);

    //shared memory
    shmdt(Shared_Memory);
    shmctl(shm_id, IPC_RMID, NULL);

    //close after, to keep the log file updated
    return(0);
}

void thread_cleanup(void *arg){
    pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);
}

void thread_cleanup_monitor(void* arg){
    pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);
}






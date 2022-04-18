#include "header.h"

int main(int argc, char *argv[]){
    
    //read config file
    readFile();

    // Criar o processo Task Manager
    initProc(taskManager, NULL);
    
    // Criar o processo Maintenance Manager
    initProc(MaintenanceManager, NULL);

    // Criar o processo Monitor
    initProc(Monitor, NULL);
    
    return 0;
}



//shared memory


// Processo que gera tarefas com um determinado nº de milhares de instruções a executar num intervalo de tempo
// Cria o named pipe onde os processos criados anteriormente irão escrever
int MobileNode()
{
}

// Função Task Manager

int TaskManager(){
    //create Edge Server processes
    for (int i = 0; i < Configs.num_servers; i++){
        initProc(EdgeServer, &Configs.servers[i]);
    }
}

// Função Maintenance Manager
int MaintenanceManager()
{
}

// Função Monitor
int Monitor()
{
}

void *vCPU(void *idp)
{
    pthread_mutex_lock(&shared_data->mutex);
    int my_id = *((int *)idp);

    sem_wait(mutex_log);
    addLog("vCPU CREATED SUCCESSFULLY");
    sem_post(mutex_log);

    pthread_mutex_unlock(&shared_data->mutex);
    pthread_exit(NULL);
    return NULL;
}

int EdgeServer(char* name, int vCPU1, int vCPU2)){
    pthread_t vCPU;
    sem_wait(mutex_write);
    shared_data->servers.name = name;
    shared_data->servers.vCPU1 = vCPU1;
    shared_data->servers.vCPU2 = vCPU2;
    sem_post(mutex_write);
    //copilot's version
    //create threads for vCPUs
    for (int i = 0; i < 2; i++){
        pthread_create(&vCPU, NULL, vCPU, &i);
    }


    //Filipe's version
    // criar vCpus (threads)
    for (int i = 0; i < 2; i++){
    
        pthread_create(&vCPU, NULL, , &shared_data->servers[i]);
    }

    // waits for threads to die
    for (int i = 0; i < shared_data->num_servers; i++){
        pthread_join(my_thread[shared_data->count - shared_data->num_servers + i], NULL);
    }
}

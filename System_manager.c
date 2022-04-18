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
int TaskManager()
{
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

    // printf("      carro %d | equipa: %d\n", my_id,getpid());
    pthread_mutex_unlock(&shared_data->mutex);
    pthread_exit(NULL);
    return NULL;
}

int EdgeServer()
{
    // criar vCpus (threads)
    for (int i = 0; i < shared_data->num_servers; i++)
    {
        sem_wait(mutex_write);
        shared_data->servers[i] = i;
        sem_post(mutex_write);

        pthread_create(&my_thread[i], NULL, carro, &shared_data->servers[i]);

        sem_wait(mutex_write);
        shared_data->count++;
        sem_post(mutex_write);
    }

    // waits for threads to die
    for (int i = 0; i < shared_data->num_servers; i++)
    {
        pthread_join(my_thread[shared_data->count - shared_data->num_servers + i], NULL);
    }
}

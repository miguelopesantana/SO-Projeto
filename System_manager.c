#include "header.h"

int main(int argc, char *argv[])
{

    if (!readFile())
        printf("Config file read correctly.\n");
    else
        printf("Error reading config file.\n");

    // Criar o processo Task Manager
    if (fork() == 0)
    {
        TaskManager();
        exit(0);
    }
    else
    {
        wait(NULL);
    }
    // Criar o processo Maintenance Manager
    if (fork() == 0)
    {
        MaintenanceManager();
        exit(0);
    }
    else
    {
        wait(NULL);
    }

    // Criar o processo Monitor
    if (fork() == 0)
    {
        Monitor();
        exit(0);
    }
    else
    {
        wait(NULL);
    }

    return 0;
}



void iniciar(){

    // remove ficheiro log, caso exista
    remove("log.txt");

    // create shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Data), IPC_CREAT | 0700)) < 0){
        perror("Error in shmget with IPC_CREAT\n");
        exit(1);
    }

    // Attach shared memory
    if ((shared_data = (Data *)shmat(shmid, NULL, 0)) == (Data *)-1){
        perror("Shmat error.\n");
        exit(1);
    }

    sem_unlink("MUTEX_LOG");                                      // remove o nome do semáforo
    mutex_log = sem_open("MUTEX_LOG", O_CREAT | O_EXCL, 0700, 1); // retorna o endereço do novo semáforo

    sem_unlink("MUTEX_WRITE");                                        // remove o nome do semáforo
    mutex_write = sem_open("MUTEX_WRITE", O_CREAT | O_EXCL, 0700, 1); // retorna o endereço do novo semáforo

    sem_wait(mutex_log); // locks the semaphore
    escreverLog("OFFLOAD SIMULATOR STARTING");
    sem_post(mutex_log); // unlocks the semaphore
}

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
    escreverLog("vCPU CREATED SUCCESSFULLY");
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

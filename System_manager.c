#include "header.h"

int main(int argc, char *argv[]){
    
    //read config file
    readFile();

    // Criar o processo Task Manager
    initProc(taskManager, NULL);
    addLog("PROCESS TASK MANAGER CREATED\n");
    
    // Criar o processo Maintenance Manager
    initProc(MaintenanceManager, NULL);
    addLog("PROCESS MAINTENANCE MANAGER CREATED\n");

    // Criar o processo Monitor
    initProc(Monitor, NULL);
    addLog("PROCESS MONITOR CREATED\n");
    
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

    //criar thread scheduler
    pthread_t scheduler;
    pthread_create(&scheduler, NULL, thread_scheduler, NULL);

    //espera que ela termine
    pthread_join(scheduler, NULL);

    exit(0);
}

// Função Maintenance Manager
int MaintenanceManager()
{
}

// Função Monitor
int Monitor()
{
}





#include "Main_header.h"

void Monitor(){
    
    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, controlMonitor, NULL);
    pthread_detach(monitor_thread);

    //wait for end signal
    pthread_mutex_t aux = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&aux);

    pthread_cond_wait(&Shared_Memory->end_system_signal, &aux);

    pthread_cancel(monitor_thread);

    pthread_mutex_unlock(&aux);
    pthread_cond_destroy(&aux);

    exit(0);
}


void controlMonitor(){
    

    while(1){

        //recebe o sinal da thread quando é colocada uma tarefa na fila
        pthread_mutex_lock(&Shared_Memory->t_queue_sem);

        pthread_cond_wait(&Shared_Memory->new_task_cond,&Shared_Memory->t_queue_sem);

        //verificar qual o modo de performance ativo
        sem_wait(&Shared_Memory->evaluate_performance_mode);

        //se a fila está ocupada a >80%, ajustar modo de performance
        if( (Shared_Memory->performance_mode == 1) && (double)Shared_Memory->task_number/ (double)Shared_Memory->num_slots > 0.8){ 

            addLog("Queue almost full: Changing performance mode to 2");

            sem_wait(Shared_Memory->evaluate_performance_mode);
            //ajuste de performance
            Shared_Memory->performance_mode = 2;

            //ajustar a disponibilidade dos vCPUs (ambos os vCPUs disponíveis)
            pthread_mutex_lock(&Shared_Memory->shm_servers);
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_servers[i].available_vCPUs[1] = 1;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_servers);

        }
        //Fila com 20% da ocupação, alterar modo de performance
        if((Shared_Memory->performance_mode == 2) && ((double)Shared_Memory->task_number / (double)Shared_Memory->num_slots < 0.2)){ 

            addLog("Changing performance mode to 1: Power saving");

            //ajuste de performance
            Shared_Memory->performance_mode = 1;

            pthread_mutex_lock(&Shared_Memory->shm_servers);
            //ajustar a disponibilidade dos vCPUs (apenas o vCPU1 disponivel)
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_servers[i].available_vCPUs[1] = 0;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_servers);
        }

        sem_post(Shared_Memory->evaluate_performance_mode);

        pthread_mutex_unlock(&Shared_Memory->t_queue_sem);

    }

    pthread_exit(NULL);
}




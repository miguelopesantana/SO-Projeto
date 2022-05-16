#include "Main_header.h"


// ----------------------------Função Monitor---------------------------------------
int workMonitor(){
    
    pthread_cleanup_push(thread_cleanup_monitor, NULL);

    while(1){
        //recebe o sinal da thread quando é colocada uma tarefa na fila
        pthread_mutex_lock(&Shared_Memory->sem_tm_queue);

        pthread_cond_wait(&Shared_Memory->new_task_cond,&Shared_Memory->sem_tm_queue);

        //printf("check monitor %d %d %f\n", SMV->node_number,SMV->QUEUE_POS, (double)SMV->node_number/ (double)SMV->QUEUE_POS);
        sem_wait(&Shared_Memory->check_performance_mode);

        if( (Shared_Memory->performance_mode == 1) && (double)Shared_Memory->node_number/ (double)Shared_Memory->num_slots > 0.8){ //se a fila está ocupada a >80%

            write_screen_log("Queue almost full: Changing performance mode to 2");

            sem_wait(Shared_Memory->check_performance_mode);
            Shared_Memory->performance_mode = 2;

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_server_lists[i].available_vCPUs[1] = 1;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);

        }

        if( (Shared_Memory->performance_mode == 2) && ( (double)Shared_Memory->node_number/ (double)Shared_Memory->num_slots < 0.2) ){ //caiu para 20% ocupação

            addLog("Changing performance mode to 1: Power saving");

            Shared_Memory->performance_mode = 1;

            pthread_mutex_lock(&Shared_Memory->shm_edge_servers);
            for(int i = 0;i< Shared_Memory->num_servers; i++){
                edge_server_lists[i].available_vCPUs[1] = 0;
            }
            pthread_mutex_unlock(&Shared_Memory->shm_edge_servers);
        }

        sem_post(Shared_Memory->check_performance_mode);

        pthread_mutex_unlock(&Shared_Memory->sem_tm_queue);

    }
    pthread_cleanup_pop(0);

    pthread_exit(NULL);

}

int Monitor(){
    //signal (SIGINT, SIG_DFL);
    pthread_t work_monitor_thread;
    pthread_create(&work_monitor_thread,NULL,workMonitor,NULL);
    pthread_detach(work_monitor_thread);

    //Wait for end signal
    pthread_mutex_t aux = PTHREAD_MUTEX_INITIALIZER; 
    pthread_mutex_lock(&aux);

    pthread_cond_wait(&SMV->end_system_sig,&aux);

    //garantir que o semaforo da condicao que vamos cancelar está unlocked
    //pthread_mutex_unlock(&SMV->sem_tm_queue);

    pthread_cancel(work_monitor_thread);

    pthread_mutex_unlock(&aux);
    pthread_mutex_destroy(&aux);

    exit(0);
}
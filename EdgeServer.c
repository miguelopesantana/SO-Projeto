#include "Main_header.h"

int EdgeServer(int num_edge_server){
    

    return 0;
}



// void * vCPU(void * arg){
//     args_cpu *t_args = (args_cpu*) arg;
    
//     //split tasks arguments with strtok
//     char *tok, *rest;
//     int task_id, num_instructions;
    
//     tok = strtok_r(t_args->task_bif, ";", &rest);
//     task_id = atoi(tok);

//     tok = strtok_r(NULL, ";", &rest);
//     num_instructions = atoi(tok);

//     //do work
//     if(t_args->cpu == 2){
//         usleep( (long long int) ( (long long int) num_instructions * 1000 / (long long int) (edge_servers[global_edge_server_number].cpu1_cap * 1000000) * 1000000) );

//     }else{
//          usleep( (long long int) ( (long long int) num_instructions * 1000 / (long long int) (edge_servers[global_edge_server_number].cpu2_cap * 1000000) * 1000000) );
//     }

//     //set cpu as available and update executed tasks
//     pthread_mutex_lock(&Shared_Memory->shm_edge_servers);

//     //check performance mode
//     sem_wait(Shared_Memory->check_performance_mode);
//     if(t_args->cpu == 2){
//         if(Shared_Memory->all_performance_mode == 2){
//             edge_servers[global_edge_server_number].available_vCPUs[t_args->cpu - 1] = 1;
//         }
//     } else{
//         edge_servers[ global_edge_server_number ].available_vCPUs[ t_args->cpu - 1] = 1;
//     }
//     sem_post(Shared_Memory->check_performance_mode);

//     edge_servers[ global_edge_server_number ].executed_tasks++;

//     //log
//     char buf[256];
//     snprintf(buf, 256, "%s,CPU%d: TASK %d COMPLETED", edge_servers_list[global_edge_server_number].name, t_args->cpu, task_id);
//     addLog(buf);

//     //Avisar o task manager/processo ES que há um CPU disponivel;
//     pthread_cond_broadcast(&Shared_Memory->edge_server_sig);
    
//     pthread_mutex_unlock(&Shared_Memory->shm_edge_servers)

//     pthread_exit(NULL);
// }
#include "Main_header.h"

// --------------------------Função Maintenance Manager----------------------------------
int MaintenanceManager(){
    signal(SIGINT,maintenance_sigint);
    pids_list = malloc(sizeof(pid_t) * Shared_Memory->num_servers);
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
        sleep(1 + rand()%5);

        server_to_maintenance = rand()%Shared_Memory->num_servers;

        //prepare message

        work_message-msgtype = pids_list[server_to_maintenance];
        work_msg.msg_content = 0;

        //write na MQ
        snprintf(buf, sizeof(buf), "Sending Edge Server %d to maintenance", server_to_maintenance);
        addLog(buf);
        msgsnd(Shared_Memory->msgqid, &work_msg, sizeof(work_msg) - sizeof(long), 0);

        //warn edge server
        pthread_cond_broadcast(&Shared_Memory->edge_server_move);

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

void maintenance_sigint(){
    free(pids_list);
    exit(0);
}
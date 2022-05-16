#include "Main_header.h"

// Função Task Manager

void TaskManager(){
    
    //open unnamed pipes
    int flags;
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        pipe(edge_servers[i].pipe);

        //set read for non-blockinng mode
        flags = fcntl(edge_servers[i].pipe[0], F_GETFL, 0);
        fcntl(edge_servers[i].pipe[0], F_SETFL, flags | O_NONBLOCK);
    }

    edge_servers_proc = malloc(sizeof(pid_t) * (Shared_Memory->num_servers));

    //Start Edge Servers
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if((edge_servers_proc[i] = fork()) == 0){
            EdgeServer(i);
            exit(0);
        }
    }

    //open named pipe for reading
    if((named_pipe_file = open(PIPE_NAME, O_RDWR)) < 0){
        addLog("Cannot open pipe");
        // end_sig_tm();
        exit(1);
    }
    addLog("Task pipe opened");

    // //Dispatcher Thread
    // pthread_create(&tm_threads[1], NULL, dispatcher, 0);

    // //Schedular Thread
    // pthread_create(&tm_threads[0], NULL, scheduler, 0);
}
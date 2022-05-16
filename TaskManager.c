#include "Main_header.h"

// Função Task Manager

int TaskManager(){
    
    //open unnamed pipes
    int flags;
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        pipe(edge_server_lists[i].pipe);

        //set read for non-blockinng mode
        flags = fcntl(edge_server_lists[i].pipe[0], F_GETFL, 0);
        fcntl(edge_server_lists[i].pipe[0], F_SETFL, flags | O_NONBLOCK);
    }

    edge_servers_processes = malloc(sizeof(pid_t) * (Shared_Memory->num_servers));

    //Start Edge Servers
    for(int i = 0; i < Shared_Memory->num_servers; i++){
        if((edge_servers_processes[i] = fork()) == 0){
            EdgeServer(i);
            exit(0);
        }
    }

    //open named pipe for reading
    if((named_pipe_file = open(PIPE_NAME, O_RDWR)) < 0){
        addLog("Cannot open pipe");
        end_sig_tm();
        exit(1);
    }
    addLog("Task pipe opened");

    //create message queue
    node_id = 0;
    msg_stack = (linked_list *)malloc(sizeof(linked_list));
    Shared_Memory->node_number = 0
    msg_stack->first_node = NULL;


    //Dispatcher Thread
    pthread_create(&tm_threads[1], NULL, dispatcher, 0);

    //Schedular Thread
    pthread_create(&tm_threads[0], NULL, scheduler, 0);

    //condiçao variável à espera que o system acabe
    endMonitorTM();

    return 0;
}
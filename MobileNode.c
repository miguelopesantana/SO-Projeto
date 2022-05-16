#include "MobileNode_header.h"


int MobileNode(int argc, char *argv[]){
    
    //ensure proper command structure
    if(argc != 5){
        error("MOBILE NODE COMMAND","mobile_node {num_requests} {interval_time} {num_commands} {max_time}");
        exit(-1);
    }else{

       if((named_pipe = open(PIPE, O_WRONLY)) < 0){
           error("MOBILE NODE PIPE","Error opening pipe");
           exit(1);
           return 1;
        }

        tasks -> num_requests = atoi(argv[1]);
        tasks -> interval_time = atoi(argv[2]);
        tasks -> num_commands = atoi(argv[3]);
        tasks -> max_time = atoi(argv[4]);

        for(int i = 0; i < tasks -> num_requests; i++){        
            if(sendRequest(tasks -> num_commands, tasks -> max_time) != 0){
                error("MOBILE NODE PIPE","Error sending request");
            }
            usleep(tasks -> interval_time);
        }
    }

    close(named_pipe);
    printf("Requests sent sucessfully\n");
    return 0;
}

int sendRequest(int num_commands, int max_time){
    char buffer[BUF_PIPE];
    srand(time(0));

    snprintf(buffer, BUF_PIPE, "%d;%d;%d\n", id++, num_commands, max_time);

    if(write(named_pipe, buffer, BUF_PIPE) < 0){
        error("MOBILE NODE PIPE","Error writing to pipe");
        return 1;
    }

    return 0;
}

int error(char *title, char *message){
    printf("[ERROR] %s: %s\n", title, message);
    return 1;
}


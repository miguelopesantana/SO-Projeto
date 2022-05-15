#include "MN_header.h"
#define BUFPIP 512
int named_pipe_file;

int main(int argc, char *argv[]){
    /*
    //ensure proper command structure
    if(argc != 7){
        error("Mobile Node command structure","mobile_node {num_requests} {interval_time} {num_commands} {max_time}");
        exit(-1);
    }

    //store the commands arguments
    float num;
    for(int i = 4; i <= 7; i++){
        if(atoi(strtok(argv[i],"{}")) <= 0){
            error("Mobile Node command values","every value must be > 0 and only times can be floats");
            exit(1);
        }else{
            switch(i){
                case 4:
                    tasks -> num_requests = atoi(strtok(argv[i],"{}"));
                    break;
                case 5:
                    tasks -> interval_time = atof(strtok(argv[i],"{}"));
                    break;
                case 6:
                    tasks -> num_commands = atoi(strtok(argv[i],"{}"));
                    break;
                case 7:
                    tasks -> max_time = atof(strtok(argv[i],"{}"));
                    break;
            }
            tasks -> id = id++;
        }
    }
    */
   if(argc == 5){
       //open TASK PIPE
       if((named_pipe_file = open(PIPE_NAME, O_RDONLY)) < 0){
           perror("Cannot open pipe for writting: ");
           exit(1);
    }
    //variables
    int time_space = atoi(argv[2]);
    int num_instructions = atoi(argv[3]);
    int timeout = atoi(argv[4]);

    for(int i = 0; i < argv[1]; i++){        
        if(generate_request(num_instructions, timeout) != 0){
            printf("Error generating request number %d\n", i);
        }
        usleep(time_space * 1000);
    }
    }else{
        fprintf(stderr, "Wrong command format\n");
        return -1;
    }

    close(named_pipe_file);
    printf("All requests placed successfully on the pipe!\nClosing mobile node...\n");
    return 0;
}

int generate_request(int num_instructions, int timeout){
    char buffer[BUFPIP];
    srand(time(0));

    snprintf(buffer, BUFPIP, "%d;%d;%d\n", 1 + rand()%500000, num_instructions, timeout);

    if(write(named_pipe_file, buffer, BUFPIP) < 0){
        perror("Write: ");
        return 1;
    }

    return 0;
}



int error(char *title, char *message){
    printf("[ERROR] %s: %s\n", title, message);
    return 1;
}


#include "MN_header.h"

int main(int argc, char *argv[]){

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
}

int error(char *title, char *message){
    printf("[ERROR] %s: %s\n", title, message);
    return 1;
}
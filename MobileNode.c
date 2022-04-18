#include "MN_header.h"

int main(int argc, char *argv[]){

    //ensure proper command structure
    if(argc != 7){
        printf("mobile_node {num_requests} {interval_time} {num_commands} {max_time}\n");
        exit(-1);
    }

    //store the command in a struct
    tasks -> num_requests = strtok(argv[4],"{}");
    tasks -> interval_time = strtok(argv[5],"{}");
    tasks -> num_commands = strtok(argv[6],"{}");
    tasks -> max_time = strtok(argv[7],"{}");
}

int error(char *title, char *message){
    addLog("[ERROR] %s: %s\n", title, message);
    return 1;
}

void addLog(char mensagem) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    FILE *file = fopen("log.txt","a");
    printf("%d:%d:%d %s\n", tm-}tm_hour, tm-}tm_min, tm-}tm_sec, mensagem);
    fprintf(file, "%d:%d:%d %s\n", tm-}tm_hour, tm-}tm_min, tm-}tm_sec, mensagem);
    fclose(file);
}
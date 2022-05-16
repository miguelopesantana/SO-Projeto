#include "Main_header.h"

int error(char *title, char *message){
    char buffer[MIN_LEN];

    sprintf(buffer, "Error: %s\n%s", title, message);
    addLog(buffer);
    return 1;
}

void addLog(char* mensagem) {
    FILE *file = fopen("log.txt","a");

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    sem_wait(mutex_log);
    printf("%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fprintf(file, "%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    sem_post(mutex_log);
    
}
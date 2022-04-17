#include "header.h"

int readFile(){

    FILE *file = fopen("config.txt", "r");
    if (file == NULL){
        printf("Error: could not open file %s\n", argv[1]);
        return 1;
    }

    char buffer[MIN_LEN];
    int num;
    char *token;

    for (i = 0; i < 2; i++){
        if (fgets(buffer, MIN_LEN, file)){
            num = atoi(buffer);
            if (num <= 0){
                switch (i){
                    case 0:
                        error("Config error", "Invalid number of queue slots");
                        break;
                    case 1:
                        error("Config error", "Invalid max waiting time");
                        break;
                    }
                fclose(file);
                exit(1);
            }else{
                switch (i){
                    case 0:
                        Config.num_slots = num;
                        break;
                    case 1:
                        Config.max_wtime = num;
                        break;
                    }
            }
        }else{
            error("Config error", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    if (fgets(buffer, MIN_LEN, file)) {
        num = atoi(buffer);
        if (num <= 2) {
            error("Config error", "Invalid number edge server (must be >=2)");
            fclose(file);
            exit(1);
        } else Configs.num_servers = num;
    } else {
        error("Config error", "Config file structure error");
        fclose(file);
        exit(1);
    }

    Configs.servers = (Server *)malloc(sizeof(Server) * Configs.num_servers);

    bool error = false;
    for (i = 0; i < Configs.num_servers; i++) {
        if (fgets(buffer, MIN_LEN, file)) {
            token = strtok(buffer, ",");
            if (strcmp(token, "") == 0) error = true;
            Configs.servers[i].name=(char*)malloc(sizeof(char*));
            strcpy(Configs.servers[i].name, token);
            for (int j = 0; j < 2; j++) {        
                token = strtok(NULL, ",");
                if(atoi(token) <= 0){
                    error = true;
                    break;
                } 
                if(j==0)Configs.servers[i].vCPU1 = atoi(token);
                if(j==1)Configs.servers[i].vCPU2 = atoi(token);                
            }
            if (error) {
                error("Config error", "Server atribute's structure error");
                fclose(file);
                exit(1);
            }            
        } else {
            error("Config error", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    fclose(file);
    return 0;
}

int error(char *title, char *message){
    addLog("%s: %s\n", title, message);
    return 1;
}

void addLog(char mensagem) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    FILE *f = fopen("log.txt","a");
    printf("%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fprintf(f, "%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fclose(f);
}
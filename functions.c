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
                        error("Config File", "Invalid number of queue slots");
                        break;
                    case 1:
                        error("Config File", "Invalid max waiting time");
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
            error("Config File", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    if (fgets(buffer, MIN_LEN, file)) {
        num = atoi(buffer);
        if (num <= 2) {
            error("Config File", "Invalid number edge server (must be >=2)");
            fclose(file);
            exit(1);
        } else Configs.num_servers = num;
    } else {
        error("Config File", "Config file structure error");
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
                error("Config File", "Server atribute's structure error");
                fclose(file);
                exit(1);
            }            
        } else {
            error("Config File", "Config file structure error");
            fclose(file);
            exit(1);
        }
    }

    fclose(file);
    return 0;
}

void initSim(){

    // remove ficheiro log, caso exista
    remove("log.txt");
    
    //internet example
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
    if (shmid == -1) {
        perror("Shared memory");
        return 1;
    }
    

    // create shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Data), IPC_CREAT | 0700)) < 0){
        error("SHM creation", "Error in shmget with IPC_CREAT\n");
        exit(1);
    }

    //internet example
    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        perror("Shared memory attach");
        return 1;
    }
    // Attach shared memory
    if ((shared_data = (Data *)shmat(shmid, NULL, 0)) == (Data *)-1){
        perror("Shmat error.\n");
        exit(1);
    }

    sem_unlink("MUTEX_LOG");                                      // remove o nome do semáforo
    mutex_log = sem_open("MUTEX_LOG", O_CREAT | O_EXCL, 0700, 1); // retorna o endereço do novo semáforo

    sem_unlink("MUTEX_WRITE");                                        // remove o nome do semáforo
    mutex_write = sem_open("MUTEX_WRITE", O_CREAT | O_EXCL, 0700, 1); // retorna o endereço do novo semáforo

    sem_wait(mutex_log); // locks the semaphore
    addLog("OFFLOAD SIMULATOR STARTING");
    sem_post(mutex_log); // unlocks the semaphore
}

//function to create a fork and execute the function with arguments
void initProc(void (*function)(), void *arg){
    if (fork() == 0){
        function(arg);
        exit(0);
    }else{
        wait(NULL);
    }
}

int error(char *title, char *message){
    addLog("[ERROR] %s: %s\n", title, message);
    return 1;
}

void addLog(char mensagem) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    FILE *file = fopen("log.txt","a");
    printf("%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fprintf(file, "%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fclose(file);
}

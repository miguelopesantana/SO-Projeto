#include "header.h"

int main(int argc, char *argv[]){

	if(!readFile(fp, users, markets)) printf("Config file read correctly.\n");
    else printf("Error reading config file.\n");
    
    //Criar o processo Task Manager
    if (fork() == 0) {
            TaskManager();
             exit(0);
    }else{
            wait(NULL);
        }
    //Criar o processo Maintenance Manager
    if (fork() == 0) {
            MaintenanceManager();
            exit(0);
    }else{
            wait(NULL);
        }

    //Criar o processo Monitor 
    if (fork() == 0) {
            Monitor();
            exit(0);
    }else{
            wait(NULL);
        }

    return 0;
}

void escreverLog(char mensagem[]) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    FILE *f = fopen("log.txt","a");
    printf("%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fprintf(f, "%d:%d:%d %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, mensagem);
    fclose(f);
}

int readFicheiro() { 

    FILE *fp = fopen("config.txt", "r");
    if(fp == NULL){
        printf("Error: could not open file %s\n", argv[1]);
        return 1;
    }

    char line[SIZE];
    int i = 0;
    int config[10];
    char *token;
    char s[3] = ", ";

    // Abertura do ficheiro "input.txt" para leitura
    FILE *file;
    file = fopen("input.txt", "r");

    // ficheiro não encontrado
    if (file == NULL) {
        sem_wait(mutex_log);
        escreverLog("FILE 'input.txt' NOT FOUND");
        sem_post(mutex_log);
        return 1;
    }

    while (fgets(line, SIZE, file) != NULL) {
        token = strtok(line,s);

        while( token != NULL ) {
            config[i] = strtol(token, &token, 10);
            token = strtok(NULL, s);
            i++;
        }
    }

    //Verificação do mínimo de 2 Edge Servers
    if (config[3] < 2) {
        sem_wait(mutex_log);
        escreverLog("NUMBER OF EDGE SERVERS NOT ENOUGH");
        sem_post(mutex_log);
        return 1;
    }
    sem_wait(mutex_write);
    shared_data -> num_slots = config[0];
    shared_data -> max_wtime = config[1];
    shared_data -> num_servers = config[2];
    int i;
    int j = 3;
    for(i = 0; i < config[2]; i++){
        shared_data -> servers[i].name = config[j];
        shared_data -> servers[i].vcpu1 = config[j+1];
        shared_data -> servers[i].vcpu2 = config[j+2];
        j++;
    }
    sem_post(mutex_write);

    fclose(file);
    return 0;
}

void iniciar() {

    //remove ficheiro log, caso exista
    remove("log.txt");

    //create shared memory
    if ((shmid = shmget(IPC_PRIVATE, sizeof(Data), IPC_CREAT | 0700)) < 0) {
        perror("Error in shmget with IPC_CREAT\n");
        exit(1);
    }

    // Attach shared memory
    if ((shared_data = (Data *) shmat(shmid, NULL, 0)) == (Data *)-1) {
        perror("Shmat error.\n");
        exit(1);
    }

    sem_unlink("MUTEX_LOG");//remove o nome do semáforo
    mutex_log = sem_open("MUTEX_LOG",O_CREAT|O_EXCL,0700,1);//retorna o endereço do novo semáforo

    sem_unlink("MUTEX_WRITE");//remove o nome do semáforo
    mutex_write = sem_open("MUTEX_WRITE",O_CREAT|O_EXCL,0700,1);//retorna o endereço do novo semáforo

    
    
    sem_wait(mutex_log);//locks the semaphore
    escreverLog("OFFLOAD SIMULATOR STARTING");
    sem_post(mutex_log);//unlocks the semaphore
}
//Processo que gera tarefas com um determinado nº de milhares de instruções a executar num intervalo de tempo
int MobileNode(){

}
//Cria o named pipe onde os processos criados anteriormente irão escrever
int CreateNamedPipe(){
    int fd;
    char * namedPipe = "C:/Users/guifa/Universidade/SO/Projeto_SO/TASK_PIPE";

    mkfifo(namedPipe, 0666);

    char arr1[80], arr2[80];

    while(1){

        fd = open(namedPipe, O_WRONLY);

        fgets(arr2, 80, stdin);

        write(fd, arr2, strlen(arr2) + 1);
        close(fd);
    }

}
//Função Task Manager
int TaskManager(){
   
}

//Função Maintenance Manager 
int MaintenanceManager(){

}

//Função Monitor
int Monitor(){

}
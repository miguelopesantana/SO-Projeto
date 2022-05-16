#include "Main_header.h"

int EdgeServer(int num_edge_server){
    
    char buffer[100];

    //colocar o cpu1 disponivel no arranque
    edge_servers[num_edge_server].available_vCPUs[0] = 1;
    

    //log
    snprintf(buffer, 100, "%s Ready", edge_servers[num_edge_server].name);
    addLog(buffer);


    return 0;
}


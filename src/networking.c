#include "networking.h"

connection_t connections[MAX_CONNECTION_COUNT] = {0};
int connectionCount = -1;

connection_t* makeConnection(int cd1, int cd2) {
    if (connectionCount < MAX_CONNECTION_COUNT){
        connection_t temp, *p;
        memset(&temp, 0, sizeof(temp));

        temp.cd1 = cd1;
        temp.cd2 = cd2;

        connections[connectionCount] = temp;
        p = &(connections[connectionCount]);
        connectionCount++;

        return p;
    } else { 
        return 0;
    }
}

void deleteConnection() {
    
}

int assignWorker(host_t hosts[MAX_HOST_NR], int clientCd) {
    for(int i = 0; i < MAX_HOST_NR; i++){
        if (hosts[i].pConnection == 0 && hosts[i].type == HOST_TYPE_WORKER) {
            hosts[i].pConnection = makeConnection(i, clientCd);
            return 0;
        }
    }

    return -1;
}
#include "networking.h"

connection_t connections[MAX_CONNECTION_COUNT] = {0};
int connectionCount = -1;

int getConnection(int cd_left, int cd_right) {
	for (int i = 0; i < connectionCount; i++){
		if 	( 	
				(connections[i].cd_left == cd_left && connections[i].cd_right == cd_right) ||
				(connections[i].cd_right == cd_left && connections[i].cd_left == cd_right) 
			) {
			return i;
		}
	}

	return -1;
}

connection_t* makeConnection(int cd_left, int cd_right) {
    if (connectionCount < MAX_CONNECTION_COUNT){

		if (getConnection(cd_left, cd_right) != -1) return 0;

        connection_t temp, *p;
        memset(&temp, 0, sizeof(temp));

        temp.cd_left = cd_left;
        temp.cd_right = cd_right;

        connections[connectionCount] = temp;
        p = &(connections[connectionCount]);
        connectionCount++;

        return p;
    } else { 
        return 0;
    }
}

void deleteConnection(int cd_left, int cd_right) {
    int connIndex = getConnection(cd_left, cd_right);

	if (connIndex == -1) return 0;

	for(int i = connIndex; i < connectionCount - 1; i++){
		connections[i] = connections[i + 1];
	}
	memset( &(connections[connectionCount - 1]), 0, sizeof(connections[connectionCount - 1]) );
	connectionCount--;
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
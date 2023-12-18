#include "svnet.h"
#include <string.h>

connection_t connections[MAX_CONNECTION_COUNT] = {0};
int connectionCount = 0;
host_t hosts[MAX_HOST_NR] = {0};

// char* getNextString(char message[MAX_BUF_SIZE]) {
// 	int firstNull = 0;

// 	for(int i = 0; i < MAX_BUF_SIZE; i++){
// 		if (firstNull == 0 && message[i] == 0){
// 			firstNull = 1;
// 		} else if (firstNull == 1 && message[i] != 0) {
// 			return message + i;
// 		}
// 	}

// 	return 0;
// }

int connWriteMsg(char input[MAX_BUF_SIZE], connection_t *connection) {
	if (connection->hasMsgToRead == 1) return -1;
	
	// DON'T USE STRCPY, msgToRead CAN CONTAIN NULL BYTE!
	memcpy(connection->msgToRead, input, MAX_BUF_SIZE);
	connection->hasMsgToRead = 1;

	return 0;
}

int connReadMsg(char output[MAX_BUF_SIZE], connection_t *connection) {
	if (connection->hasMsgToRead == 0) return -1;
	
	// DON'T USE STRCPY, msgToRead CAN CONTAIN NULL BYTE!
	memcpy(output, connection->msgToRead, MAX_BUF_SIZE);
	memset(connection->msgToRead, 0, MAX_BUF_SIZE);
	connection->hasMsgToRead = 0;

	return 0;
}

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
            hosts[i].pConnection = makeConnection(clientCd, i);
			hosts[i].pConnection->direction = CONN_LEFTRIGHT;
			hosts[clientCd].pConnection = hosts[i].pConnection;
            return 0;
        }
    }

    return -1;
}

int isSender(int cd, connection_t conn) {
	for(int i = 0; i < connectionCount; i++){
		if ((conn.cd_left	== cd && conn.direction == CONN_LEFTRIGHT) ||
			(conn.cd_right 	== cd && conn.direction == CONN_RIGHTLEFT)	) 
		{
			return 1;
		}
	}

	return 0;
}

int isReceiver(int cd, connection_t conn) {
    return !isSender(cd, conn);
}
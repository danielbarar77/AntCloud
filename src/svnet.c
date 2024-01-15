#include "svnet.h"
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

connection_t connections[MAX_CONNECTION_COUNT] = {0};
int connectionCount = 0;
int hostCount = 0;
host_t hosts[MAX_HOST_NR] = {0};

int connWriteMsg(char input[MAX_BUF_SIZE], int msgSize, connection_t *connection) {
	if (connection->hasMsgToRead == 1) return -1;
	if (msgSize <= 0) return 0;
	
	// DON'T USE STRCPY, msgToRead CAN CONTAIN NULL BYTE!
	memset(connection->msgToRead, 0, MAX_BUF_SIZE);
	memcpy(connection->msgToRead, input, msgSize);
	connection->hasMsgToRead = 1;
	connection->msgSize = msgSize;

	return 0;
}

int connReadMsg(char output[MAX_BUF_SIZE], connection_t *connection) {
	if (connection->hasMsgToRead == 0) return -1;
	
	// DON'T USE STRCPY, msgToRead CAN CONTAIN NULL BYTE!
	memcpy(output, connection->msgToRead, connection->msgSize);
	memset(connection->msgToRead, 0, MAX_BUF_SIZE);
	connection->hasMsgToRead = 0;
	int rez = connection->msgSize;
	connection->msgSize = 0;

	return rez;
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

void deleteHostConnection(int cd){
	if (hosts[cd].pConnection == NULL) return;

	int cd_left;
	int cd_right;

	if (hosts[cd].pConnection->cd_left == cd) 
	{
		cd_left = cd;
		cd_right = hosts[cd].pConnection->cd_right;
	}
	else 
	{
		cd_left = hosts[cd].pConnection->cd_left;
		cd_right = cd;
	}

	hosts[cd_left].pConnection = NULL;
	hosts[cd_right].pConnection = NULL;

	deleteConnection(cd_left, cd_right);

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

void deleteHost(int cd)
{
	memset(&(hosts[cd]), 0, sizeof(host_t));
	hostCount--;
}

void *getIpAddress(int cd, char *ipAddr)
{
	struct sockaddr clientSa;
	struct sockaddr_in* clientSaIn = (struct sockaddr_in*) &clientSa;
	socklen_t clientSaLen;
	memset(&clientSa, 0, sizeof(clientSa));
	getpeername(cd, &clientSa, &clientSaLen);
	strcpy(ipAddr, inet_ntoa(clientSaIn->sin_addr));

	return;
}

int assignWorker(int clientCd) {
    for(int i = 0; i < MAX_HOST_NR; i++){
        if (hosts[i].pConnection == 0 && hosts[i].type == HOST_TYPE_WORKER) {
            hosts[i].pConnection = makeConnection(clientCd, i);
			//hosts[i].pConnection->direction = CONN_LEFTRIGHT;
			hosts[clientCd].pConnection = hosts[i].pConnection;
            return 0;
        }
    }

    return -1;
}
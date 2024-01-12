#ifndef NETWORKING_H
#define NETWORKING_H

#include "common.h"

#define MAX_CONNECTION_QUEUE 5

#define MAX_HOST_NR 20
#define MAX_CONNECTION_COUNT 5

enum HOST_TYPE {
    HOST_TYPE_NULL,
    HOST_TYPE_WORKER,
    HOST_TYPE_CLIENT
};

typedef enum HOST_TYPE host_type_t;

enum HOST_ROLE {
    HOST_ROLE_NULL,
    HOST_ROLE_SENDER,
    HOST_ROLE_RECEIVER
};

typedef enum HOST_ROLE host_role_t;

struct connection;
typedef struct connection connection_t;

struct host {
    host_type_t type;
    connection_t* pConnection;
};

struct connection {
    int cd_left;
    int cd_right;
	char msgToRead[MAX_BUF_SIZE];
	int hasMsgToRead;
    int msgSize;
};

typedef struct host host_t;

extern connection_t connections[MAX_CONNECTION_COUNT];
extern int connectionCount;
extern int hostCount;

extern host_t hosts[MAX_HOST_NR]; // hosts[socket_conn] = type of the host connected on the socket

// /// @brief  Finds the next string in a message containing multiple null terminated strings.
// /// @param  message
// /// @return Pointer to the next string, if exists. 0 if not.
// char* getNextString(char message[MAX_BUF_SIZE]);

/// @brief 	Writes the message with size msgSize from input on the connection.
///			If the connection already has a message that has not been read yet, it fails.
///	@param	input
///	@param	msgSize
///	@param	connection
///	@return	0 if successful, -1 otherwise
int connWriteMsg(char input[MAX_BUF_SIZE], int msgSize, connection_t *connection);

/// @brief 	Reads the message on the connection onto output. 
///			If the connection doesn't have a message to be read, it fails.
///	@param	input
///	@param	connection
///	@return	Number of bytes read, if successful, -1 otherwise
int connReadMsg(char output[MAX_BUF_SIZE], connection_t *connection);

/// @param cd_left
/// @param cd_right
/// @return Returns the index of the connection between cd_left and cd_right if it exists, -1 otherwise
int getConnectionIndex(int cd_left, int cd_right);

/// @brief Establishes a connection between the two hosts. If the connection already exists, fails.
/// @param cd_left 
/// @param cd_right 
/// @return Pointer to connection if successful, NULL otherwise
connection_t* makeConnection(int cd_left, int cd_right);

/// @brief Deletes connection between sockets cd_left and cd_right
/// @param cd_left
/// @param cd_right
/// @return void
void deleteConnection(int cd_left, int cd_right);

/// @brief Deletes connection of host with connection descriptor cd in hosts
/// @param cd
/// @return void
void deleteHostConnection(int cd);

/// @brief  Searches in hosts for available worker. If it find one, sets the client's otherCd to the host, 
//          and the host's otherCd to the client. The clientCd must be in hosts, otherwise the function will fail
/// @param  host_t hosts[MAX_HOST_NR], int clientCd 
/// @return 0 if successful, -1 otherwise 
int assignWorker(int clientCd);

/// @brief Deletes host information about socket with descriptor cd
/// @param cd 
void deleteHost(int cd);

#endif
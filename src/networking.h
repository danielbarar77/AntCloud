#ifndef NETWORKING_H
#define NETWORKING_H

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

enum CONN_DIRECTION {
	CONN_LEFTRIGHT,
	CONN_RIGHTLEFT
};

typedef enum CONN_DIRECTION conn_direction_t;

struct connection;
typedef struct connection connection_t;

struct host {
    host_type_t type;
    connection_t* pConnection;
	conn_direction_t direction;
};

struct connection {
    int cd_left;
    int cd_right;
};

typedef struct host host_t;

extern connection_t connections[MAX_CONNECTION_COUNT];
extern int connectionCount;

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

/// @brief  Searches in hosts for available worker. If it find one, sets the client's otherCd to the host, 
//          and the host's otherCd to the client. The clientCd must be in hosts, otherwise the function will fail
/// @param  host_t hosts[MAX_HOST_NR], int clientCd 
/// @return 0 if successful, -1 otherwise 
int assignWorker(host_t hosts[MAX_HOST_NR], int clientCd);

#endif
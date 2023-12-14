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
    int cd1;
    int cd2;
};

typedef struct host host_t;

extern connection_t connections[MAX_CONNECTION_COUNT];
extern int connectionCount;

/// @brief Establishes a connection between the two hosts
/// @param cd1 
/// @param cd2 
/// @return Pointer to connection if successful, NULL otherwise
connection_t* makeConnection(int cd1, int cd2);

/// @brief Deletes connection between sockets
void deleteConnection();

/// @brief  Searches in hosts for available worker. If it find one, sets the client's otherCd to the host, 
//          and the host's otherCd to the client. The clientCd must be in hosts, otherwise the function will fail
/// @param  host_t hosts[MAX_HOST_NR], int clientCd 
/// @return 0 if successful, -1 otherwise 
int assignWorker(host_t hosts[MAX_HOST_NR], int clientCd);

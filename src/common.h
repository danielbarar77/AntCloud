#define CONNECTION_ACCEPTED_STR "Connection accepted!"
#define MAX_BUF_SIZE 1000

typedef enum MSG_TYPE{
    MSG_TYPE_COMMAND,
    MSG_TYPE_INFO
};

typedef enum CMD_TYPE{
    CMD_ADD
};

typedef enum HOST_TYPE{
    HOST_TYPE_WORKER,
    HOST_TYPE_CLIENT
};

typedef enum INFO_CODES{
    INFO_CONNECTION_ACCEPTED
};
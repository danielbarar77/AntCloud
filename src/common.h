#define CMD_RUN "run"
#define CMD_RETURN "return"

#define CLIENT_GREETING "client\n"
#define WORKER_GREETING "worker\n"
#define CONNECTION_ACCEPTED_STR "Connection accepted!"
#define END_TRANSMISSION_SIGNAL "END_TRANSMISSION"
#define MAX_BUF_SIZE 1000
#define MAX_PROGRAM_SIZE MAX_BUF_SIZE - sizeof(CMD_RUN)
#define MAX_OUTPUT_SIZE MAX_BUF_SIZE - sizeof(CMD_RETURN)
#define MAX_WORKER_NR 10
#define MAX_CLIENT_NR 10
#define MAX_CMD_LENGTH 10
#define MAX_ARGS 16
#define ARGS_LENGTH 64
#define ARGUMENTS_SIGNAL "ARGUMENTS_SIGNAL"

typedef struct arguments
{
	char **args;
	int argc;
} arguments;

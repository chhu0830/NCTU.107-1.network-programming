#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 16384
#define MAX_PIPE_NUM 4096
#define MAX_FILENAME_LENGTH 1024
#define UNKNOWN_COMMAND_ERRNO -1

struct CMD {
    char cmd[MAX_COMMAND_LENGTH];
    char **argv;
    int argc, pid;
};

struct PROCESS {
    struct CMD cmds[MAX_PIPE_NUM];
    int input, output, error, count, num, redirect_error, userin, userout;
    char filename[MAX_FILENAME_LENGTH], fifo[32], cmd[MAX_INPUT_LENGTH], **env;
};

void SIGCHLD_HANDLER();
void parse_pipe(struct PROCESS *process, char *buf);
void parse_redirect(struct PROCESS *process);
int parse_args(struct PROCESS *process);
int set_io(struct PROCESS *process, struct USER *user);
void shell(struct PROCESS *process);
void move_numfd(int (*numfd)[2]);
void free_process(struct PROCESS *process);
int read_until_newline(int fd, char *buf);
int npshell(struct USER *user, char *buf);

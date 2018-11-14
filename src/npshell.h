#define MAX_COMMAND_LENGTH 256
#define MAX_INPUT_LENGTH 16384
#define MAX_PIPE_NUM 4096
#define MAX_NUMBERED_PIPE 1024
#define MAX_FILENAME_LENGTH 1024
#define UNKNOWN_COMMAND_ERRNO -1

struct CMD {
    char cmd[MAX_COMMAND_LENGTH];
    char **argv;
    int argc, pid;
};

struct PROCESS {
    struct CMD cmds[MAX_PIPE_NUM];
    int input, output, error, count, num, redirect_error;
    char filename[MAX_FILENAME_LENGTH];
};

struct USER;

void parse_pipe(struct PROCESS *process, char *buf);
void parse_redirect(struct PROCESS *process);
int parse_args(struct PROCESS *process);
int exec(struct PROCESS *process);
void set_io(struct PROCESS *process, int (*numfd)[2], int sockfd);
void shell(struct PROCESS *process);
void move_numfd(int (*numfd)[2]);
void free_process(struct PROCESS *process);
int read_until_newline(int fd, char *buf);
int npshell(struct USER *user, char *buf, int (*numfd)[2]);

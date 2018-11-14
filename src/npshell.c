#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include "user.h"
#include "npshell.h"

void parse_pipe(struct PROCESS *process, char *buf)
{
    int i;
    char *cmd;
    for (i = 0; (cmd = strsep(&buf, "|")); i++) {
        strcpy(process->cmds[i].cmd, cmd);
    }
    process->count = i;
}

void parse_redirect(struct PROCESS *process)
{
    int len;
    char *ptr, *cmd = process->cmds[0].cmd;
    if ((ptr = strchr(cmd, '<')) != NULL && isdigit(*(ptr+1))) {
        sscanf(ptr+1, "%d%n", &process->userin, &len);
        memset(ptr, ' ', len+1);
    }

    cmd = process->cmds[process->count - 1].cmd;
    if ((ptr = strchr(cmd, '!')) != NULL) {
        sscanf(ptr+1, "%d", &process->num);
        process->redirect_error = 1;
        *ptr = '\0';
    }
    else if ((ptr = strchr(cmd, '>')) != NULL) {
        if (*(ptr+1) == ' ') {
            sscanf(ptr+1, "%s", process->filename);
            *ptr = '\0';
        } else if (isdigit(*(ptr+1))) {
            sscanf(ptr+1, "%d", &process->userout);
            *ptr = '\0';
        }
    } else if (process->count != 1 && isdigit(cmd[0])) {
        sscanf(cmd, "%d", &process->num);
        process->count -= 1;
    }
}

int parse_args(struct PROCESS *process)
{
    char *cmd;
    for (int i = 0; i < process->count; i++) {
        wordexp_t exp;
        cmd = process->cmds[i].cmd;

        wordexp(cmd, &exp, 0);
        if (exp.we_wordc == 0) {
            wordfree(&exp);
            return -1;
        }

        process->cmds[i].argc = exp.we_wordc;
        process->cmds[i].argv = calloc(sizeof(char*), (exp.we_wordc+1));

        for (unsigned int j = 0; j < exp.we_wordc; j++) {
            process->cmds[i].argv[j] = strdup(exp.we_wordv[j]);
        }
        wordfree(&exp);
    }
    return 0;
}

void set_io(struct PROCESS *process, int (*numfd)[2], int sockfd)
{
    if (numfd[0][0]) {
        process->input = numfd[0][0];
        close(numfd[0][1]);
    } else {
        process->input = dup(sockfd);
    }

    if (process->filename[0]) {
        process->output = open(process->filename, O_WRONLY|O_TRUNC|O_CREAT|O_CLOEXEC, 0666);
    } else if (process->num) {
        if (numfd[process->num][1] == 0) {
            while (pipe2(numfd[process->num], O_CLOEXEC) < 0);
        }
        process->output = dup(numfd[process->num][1]);
    } else {
        process->output = dup(sockfd);
    }
    process->error = dup(sockfd);
}

void shell(struct PROCESS *process)
{
    int pid, curfd[2], prefd = process->input;
    const int LST = process->count-1;

    for (int i = 0; i < process->count; i++) {
        while (pipe2(curfd, O_CLOEXEC) < 0);
        while ((process->cmds[i].pid = pid = fork()) < 0);
        if (pid == 0) {
            dup2(prefd, 0);
            dup2((i == LST) ? process->output : curfd[1], 1);
            dup2(process->error, 2);
            if (i == LST && process->redirect_error) dup2(process->output, 2);

            if (execvp(process->cmds[i].argv[0], process->cmds[i].argv) == -1) {
                fprintf(stderr, "Unknown command: [%s].\n", process->cmds[i].argv[0]);
                exit(UNKNOWN_COMMAND_ERRNO);
            }
            exit(0);
        } else {
            close(prefd);
            close(curfd[1]);
            prefd = curfd[0];
        }
    } 

    close(prefd);
    close(process->output);
    close(process->error);

    if (process->num == 0) {
        for (int i = 0, status; i < process->count; i++) {
            waitpid(process->cmds[i].pid, &status, 0);
        }
    }
}

void move_numfd(int (*numfd)[2])
{
    for (int i = 1; i < MAX_NUMBERED_PIPE; i++) {
        numfd[i-1][0] = numfd[i][0];
        numfd[i-1][1] = numfd[i][1];
    }
}

void free_process(struct PROCESS *process)
{
    for (int i = 0; i < process->count; i++) {
        for (int j = 0; j < process->cmds[i].argc; j++) {
            free(process->cmds[i].argv[j]);
        }
        free(process->cmds[i].argv);
    }
    close(process->input);
    close(process->output);
    close(process->error);
}


int read_until_newline(int fd, char *buf)
{
    int len = 0;

    while (read(fd, buf+len, 1) > 0) {
        if (buf[len++] == '\n') {
            len = strcspn(buf, "\r\n");
            buf[len] = '\0';
            return len;
        }
    }

    return -1;
}

int npshell(struct USER *users, struct USER *user, char *buf)
{
    int status = 0, len;
    char cmd[MAX_COMMAND_LENGTH], argv1[MAX_COMMAND_LENGTH], argv2[MAX_COMMAND_LENGTH];
    char *ptr = buf;
    sscanf(ptr, "%s%n", cmd, &len);
    ptr += len + 1;

    if (strcmp(cmd, "exit") == 0) {
        status = -1;
    } else if (strcmp(cmd, "setenv") == 0) {
        sscanf(ptr, "%s %s", argv1, argv2);
        setenv(argv1, argv2, 1);
    } else if (strcmp(cmd, "printenv") == 0) {
        sscanf(ptr, "%s", argv1);
        if ((ptr = getenv(argv1)) != NULL) {
            dprintf(user->sockfd, "%s\n", ptr);
        }
    } else if (strcmp(cmd, "who") == 0) {
        who(users, user);
    } else if (strcmp(cmd, "name") == 0) {
        name(users, user, ptr);
    } else if (strcmp(cmd, "tell") == 0) {
        sscanf(ptr, "%s%n", argv1, &len);
        ptr += len + 1;
        tell(users, user, atoi(argv1), ptr);
    } else if (strcmp(cmd, "yell") == 0) {
        yell(users, user, ptr);
    } else {
        struct PROCESS process;
        memset(&process, 0, sizeof(struct PROCESS));

        parse_pipe(&process, buf);
        parse_redirect(&process);
        if (parse_args(&process) == 0) {
            set_io(&process, user->numfd, user->sockfd);
            shell(&process);
            free_process(&process);
        }
    }

    move_numfd(user->numfd);

    return status;
}

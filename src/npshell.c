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
    char *ptr, *cmd = process->cmds[process->count - 1].cmd;

    if ((ptr = strchr(cmd, '>'))) {
        *ptr = '\0';
        sscanf(ptr+1, "%s", process->filename);
    } else if ((ptr = strchr(cmd, '!'))) {
        *ptr = '\0';
        sscanf(ptr+1, "%d", &process->num);
        process->redirect_error = 1;
    } else if (process->count != 1 && isdigit(cmd[0])) {
        char *ptr, *buf = cmd;
        for (int i = 0, num; (ptr = strsep(&buf, "+")); i++) {
            sscanf(ptr, "%d", &num);
            process->num += num;
        }
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

int exec(struct PROCESS *process)
{
    struct CMD *cmd = &process->cmds[0];
    char *ptr;
    if (strcmp(cmd->argv[0], "exit") == 0) {
        return -1;
    } else if (strcmp(cmd->argv[0], "setenv") == 0) {
        setenv(cmd->argv[1], cmd->argv[2], 1);
    } else if (strcmp(cmd->argv[0], "printenv") == 0) {
        if ((ptr = getenv(cmd->argv[1]))) {
            strcat(ptr, "\n");
            write(process->output, ptr, strlen(ptr));
        }
    } else {
        shell(process);
        return 1;
    }
    return 0;
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

int npshell(struct USER *user, char *buf, int (*numfd)[2])
{
    struct PROCESS process;
    memset(&process, 0, sizeof(struct PROCESS));

    parse_pipe(&process, buf);
    parse_redirect(&process);
    if (parse_args(&process) < 0) return 1;

    set_io(&process, numfd, user->sockfd);
    int status = exec(&process);
    free_process(&process);

    if (status < 0) {
        return status;
    }

    move_numfd(numfd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/shm.h> 
#include "user.h"

void init_users()
{
    const size_t SIZE = sizeof(struct USER) * MAX_USER_NUM;
#if defined(SIMPLE) || defined(SINGLE)
    users = (struct USER*)malloc(SIZE);
#elif defined(MULTI)
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT|O_RDWR, 0666); 
    ftruncate(shm_fd, SIZE);
    users = (struct USER*)mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
#endif
    set_users();
}

void free_users()
{
    clear_users();
#if defined(SIMPLE) || defined(SINGLE)
    free(users);
#elif defined(MULTI)
    munmap(users, MAX_USER_NUM);
    shm_unlink(SHARED_MEMORY_NAME);
#endif
}

void set_users()
{
    memset(users, 0, sizeof(struct USER) * MAX_USER_NUM);
    for (int i = 0; i < MAX_USER_NUM; i++) {
        set_user(&users[i], i+1);
    }
}

void set_user(struct USER *user, int id)
{
    user->id = id;
    strcpy(user->name, "(no name)");
    user->env = (char**)calloc(1, sizeof(char*));
    npsetenv(user, "PATH", "bin:.");
}

void clear_users()
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        clear_user(&users[i]);
    }
}

void clear_user(struct USER *user)
{
    if (user->sockfd > 2) {
        close(user->sockfd);
    }

    for (int i = 0; i < MAX_NUMBERED_PIPE; i++) {
        if (user->numfd[i][0] > 2) {
            close(user->numfd[i][0]);
        }
        if (user->numfd[i][1] > 2) {
            close(user->numfd[i][1]);
        }
    }

    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].userfd[user->id-1] > 2) {
            users[i].userctl[user->id-1] = 2;
            kill(users[i].pid, SIGUSR2);
        }
        if (user->userfd[i] > 2) {
            close(user->userfd[i]);
        }
    }

    if (user->env != NULL) {
        for (int i = 0; user->env[i]; i++) {
            free(user->env[i]);
        }
        free(user->env);
    }

    memset(user, 0, sizeof(struct USER));
}

struct USER* available_user()
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd == 0) {
            return &users[i];
        }
    }
    return NULL;
}

void npsetenv(struct USER *user, char *key, char *value)
{
    int i;
    char buf[MAX_ENV_LENGTH];
    for (i = 0; user->env[i]; i++) {
        if (strstr(user->env[i], key) != NULL) {
            strcpy(strchr(user->env[i], '=')+1, value);
            return;
        }
    }
    user->env = realloc(user->env, sizeof(char*) * (i+2));
    sprintf(buf, "%s=%s", key, value);
    user->env[i] = strdup(buf);
    user->env[i+1] = NULL;
}

void npgetenv(struct USER *user, char *key)
{
    for (int i = 0; user->env[i]; i++) {
        if (strstr(user->env[i], key) != NULL) {
            dprintf(user->sockfd, "%s\n", strchr(user->env[i], '=')+1);
            return;
        }
    }
}

void leave(struct USER *user)
{
#if defined(SINGLE) || defined(MULTI)
    broadcast_msg("*** User '%s' left. ***\n", user->name);
#endif
    int id = user->id;
    clear_user(user);
    set_user(user, id);
}

void who(struct USER *user)
{
    dprintf(user->sockfd, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            dprintf(user->sockfd, "%d\t%s\t%s/%d%s", users[i].id, users[i].name, users[i].ip, users[i].port, (users[i].id == user->id) ? "\t<-me\n" : "\n");
        }
    }
}

void name(struct USER *user, char *name)
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (strcmp(users[i].name, name) == 0) {
            dprintf(user->sockfd, "*** User '%s' already exists. ***\n", name);
            return;
        }
    }
    strcpy(user->name, name);
    broadcast_msg("*** User from %s/%d is named '%s'. ***\n", user->ip, user->port, user->name);
}

void tell(struct USER *user, int id, char *buf)
{
    if (users[id-1].sockfd == 0) {
        dprintf(user->sockfd, "*** Error: user #%d does not exist yet. ***\n", id);
    } else {
        send_msg(&users[id-1], "*** %s told you ***: %s\n", user->name, buf);
    }
}

void yell(struct USER *user, char *buf)
{
    broadcast_msg("*** %s yelled ***: %s\n", user->name, buf);
}

void welcome_msg(struct USER *user)
{
    dprintf(user->sockfd, "****************************************\n");
    dprintf(user->sockfd, "** Welcome to the information server. **\n");
    dprintf(user->sockfd, "****************************************\n");
}

void send_msg(struct USER *user, char *format, ...)
{
    va_list args;
    va_start(args, format);
#if defined(MULTI)
    if (user->pid != 0) {
        while (user->msg[0] != '\0');
        vsprintf(user->msg, format, args);
        kill(user->pid, SIGUSR1);
    }
#elif defined(SIMPLE) || defined(SINGLE)
    vdprintf(user->sockfd, format, args);
#endif
    va_end(args);
}

void broadcast_msg(char *format, ...)
{
    va_list args, copy;
    va_start(args, format);
    va_copy(copy, args);
    char *str = malloc(vsnprintf(NULL, 0, format, args) + 1);
    vsprintf(str, format, copy);
    va_end(args);
    va_end(copy);

    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            send_msg(&users[i], str);
        }
    }
    
    free(str);
}

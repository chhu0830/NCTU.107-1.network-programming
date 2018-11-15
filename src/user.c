#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/shm.h> 
#include "user.h"

struct USER* init_users()
{
    const size_t SIZE = sizeof(struct USER) * MAX_USER_NUM;
#if defined(SIMPLE) || defined(SINGLE)
    struct USER *users = (struct USER*)malloc(SIZE);
#elif defined(MULTI)
    int shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666); 
    ftruncate(shm_fd, SIZE);
    struct USER *users = (struct USER*)mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
#endif
    reset_users(users);
    return users;
}

void free_users(struct USER *users)
{
#if defined(SIMPLE) || defined(SINGLE)
    free(users);
#elif defined(MULTI)
    munmap(users, MAX_USER_NUM);
    shm_unlink(SHARED_MEMORY_NAME);
#endif
}

void reset_users(struct USER *users)
{
    memset(users, 0, sizeof(struct USER) * MAX_USER_NUM);
    for (int i = 0; i < MAX_USER_NUM; i++) {
        users[i].id = i + 1;
        strcpy(users[i].name, "(no name)");
        users[i].env = (char**)calloc(1, sizeof(char*));
        npsetenv(&users[i], "PATH", "bin:.");
    }
}

void reset_user(struct USER *user)
{
    int id = user->id;
    close(user->sockfd);
    for (int i = 0; user->env[i]; i++) {
        free(user->env[i]);
    }
    free(user->env);
    memset(user, 0, sizeof(struct USER));
    user->id = id;
    strcpy(user->name, "(no name)");
    user->env = (char**)calloc(1, sizeof(char*));
    npsetenv(user, "PATH", "bin:.");
}

struct USER* available_user(struct USER *users)
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

void leave(struct USER *users, struct USER *user)
{
#if defined(SINGLE) || defined(MULTI)
    char msg[MAX_MSG_LENGTH];
    sprintf(msg, "*** User '%s' left. ***", user->name);
    broadcast_msg(users, msg);
#endif
    reset_user(user);
}

void who(struct USER *users, struct USER *user)
{
    dprintf(user->sockfd, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            dprintf(user->sockfd, "%d\t%s\t%s/%d%s", users[i].id, users[i].name, users[i].ip, users[i].port, (users[i].id == user->id) ? "\t<-me\n" : "\n");
        }
    }
}

void name(struct USER *users, struct USER *user, char *name)
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (strcmp(users[i].name, name) == 0) {
            dprintf(user->sockfd, "*** User '%s' already exists. ***\n", name);
            return;
        }
    }
    char msg[MAX_MSG_LENGTH];
    strcpy(user->name, name);
    sprintf(msg, "*** User from %s/%d is named '%s'. ***", user->ip, user->port, user->name);
    broadcast_msg(users, msg);
}

void tell(struct USER *users, struct USER *user, int id, char *buf)
{
    if (users[id-1].sockfd == 0) {
        dprintf(user->sockfd, "*** Error: user #%d does not exist yet. ***\n", id);
    } else {
        char msg[MAX_MSG_LENGTH];
        sprintf(msg, "*** %s told you ***: %s", user->name, buf);
        send_msg(&users[id-1], msg);
    }
}

void yell(struct USER *users, struct USER *user, char *buf)
{
    char msg[MAX_MSG_LENGTH];
    sprintf(msg, "*** %s yelled ***: %s", user->name, buf);
    broadcast_msg(users, msg);
}

void send_msg(struct USER *user, char *msg)
{
#if defined(MULTI)
    if (user->pid != 0) {
        strcpy(user->msg, msg);
        kill(user->pid, SIGUSR1);
    }
#elif defined(SINGLE)
    dprintf(user->sockfd, "%s\n", msg);
#endif
}

void broadcast_msg(struct USER *users, char *msg)
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            send_msg(&users[i], msg);
        }
    }
}

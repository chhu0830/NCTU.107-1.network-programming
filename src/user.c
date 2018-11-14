#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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
    for (int i = 0; i < MAX_USER_NUM; i++) {
        memset(&users[i], 0, sizeof(struct USER));
        users[i].id = i + 1;
        strcpy(users[i].name, "(no name)");
    }
}

void reset_user(struct USER *user)
{
    int id = user->id;
    close(user->sockfd);
    memset(user, 0, sizeof(struct USER));
    user->id = id;
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

void who(struct USER *users, struct USER *user)
{
    send_msg(user, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            char msg[MAX_NAME_LENGTH + 64];
            sprintf(msg, "%d\t%s\t%s/%d%s", users[i].id, users[i].name, users[i].ip, users[i].port, (users[i].id == user->id) ? "\t<- me\n" : "\n");
            send_msg(user, msg);
        }
    }
}

void name(struct USER *user, char *name)
{
    strcpy(user->name, name);
}

void send_msg(struct USER *user, char *msg)
{
    write(user->sockfd, msg, strlen(msg));
}

void broadcast_msg(struct USER *users, char *msg)
{
    for (int i = 0; i < MAX_USER_NUM; i++) {
        if (users[i].sockfd != 0) {
            send_msg(&users[i], msg);
        }
    }
}

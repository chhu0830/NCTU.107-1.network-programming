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

void free_users(struct USER* users)
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
        users[i].id = i;
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

void send_msg(struct USER *user, char *msg)
{
    write(user->sockfd, msg, strlen(msg));
}

#include <string.h>
#include <unistd.h>
#include "user.h"

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

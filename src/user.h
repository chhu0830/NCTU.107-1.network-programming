#define MAX_NAME_LENGTH 256
#define MAX_USER_NUM 32
#define MAX_NUMBERED_PIPE 1024
#define SHARED_MEMORY_NAME "RWG"

struct USER {
    char name[MAX_NAME_LENGTH], ip[32];
    int numfd[MAX_NUMBERED_PIPE][2];
    int id, sockfd, port;
};

struct USER* init_users();
void free_users(struct USER *users);
void reset_users(struct USER *users);
void reset_user(struct USER *user);
struct USER* available_user(struct USER *users);
void who(struct USER *users, struct USER *user);
void name(struct USER *user, char *name);
void send_msg(struct USER *user, char *msg);
void broadcast_msg(struct USER *users, char *msg);

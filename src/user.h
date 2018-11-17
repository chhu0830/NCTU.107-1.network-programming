#define MAX_NAME_LENGTH 32
#define MAX_MSG_LENGTH 2048
#define MAX_USER_NUM 32
#define MAX_ENV_LENGTH 256
#define MAX_NUMBERED_PIPE 1024
#define SHARED_MEMORY_NAME "RWG"

struct USER {
    char name[MAX_NAME_LENGTH], ip[32], msg[MAX_MSG_LENGTH], **env;
    int numfd[MAX_NUMBERED_PIPE][2], userfd[MAX_USER_NUM];
    int id, sockfd, port, pid;
};

#ifndef USRS
#define USRS
struct USER *users;
#endif

void RECV_MSG();
void init_users() __attribute__((constructor));
void free_users();
void set_users();
void set_user(struct USER *user, int id);
void clear_users();
void clear_user(struct USER *user);
struct USER* available_user();
void npsetenv(struct USER *user, char *key, char *value);
void npgetenv(struct USER *user, char *key);
void leave(struct USER *user);
void who(struct USER *user);
void name(struct USER *user, char *name);
void tell(struct USER *user, int id, char *buf);
void yell(struct USER *user, char *buf);
void welcome_msg(struct USER *user);
void send_msg(struct USER *user, char *msg);
void broadcast_msg(char *msg);

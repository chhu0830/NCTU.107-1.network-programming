#define MAX_NAME_LENGTH 256
#define MAX_USER_NUM 1

struct USER {
   char name[MAX_NAME_LENGTH], addr[32];
   int id, sockfd, port;
};

void reset_users(struct USER *users);
void reset_user(struct USER *user);
struct USER* available_user(struct USER *users);

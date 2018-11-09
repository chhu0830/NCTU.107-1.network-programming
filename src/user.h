#define MAX_NAME_LENGTH 256

struct USER {
   char name[MAX_NAME_LENGTH], addr[32];
   int id, sockfd, port;
};

void reset_user(struct USER *user, int id);

struct USER;

int create_socket();
int listen_socket(int sockfd, const char *HOST, const int PORT);
int accept_client(int sockfd, struct USER *user);
void welcome_message(struct USER *user);

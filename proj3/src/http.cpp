#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <boost/asio.hpp>
#include "http_server.hpp"

using namespace std;

extern char **environ;
extern boost::asio::io_service global_io_service;

void SIGCHLD_HANDLER(int);

int main(int argc, const char *argv[])
{
    signal(SIGCHLD, SIGCHLD_HANDLER);

    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " [port]" << endl;
        exit(1);
    }

    try {
        short port = atoi(argv[1]);
        Server server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}

void SIGCHLD_HANDLER(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

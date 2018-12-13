#include <iostream>
#include <vector>
#include <sstream>
#include <boost/asio.hpp>
#include "http_server.hpp"

using namespace std;

extern char **environ;
extern boost::asio::io_service global_io_service;

int main(int argc, const char *argv[])
{
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

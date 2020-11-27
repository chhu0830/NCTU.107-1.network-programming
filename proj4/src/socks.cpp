#include <iostream>
#include <boost/asio.hpp>
#include "socks_server.hpp"

using namespace std;
using namespace boost::asio;

extern io_service global_io_service;

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " [PORT]" << endl;
        exit(1);
    }

#ifndef DEBUG
    close(STDERR_FILENO);
#endif

    try {
        unsigned short port = atoi(argv[1]);
        Server Server(port);
        global_io_service.run();
    } catch (exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}

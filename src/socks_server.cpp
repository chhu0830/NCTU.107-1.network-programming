#include "socks_server.hpp"
#include <iostream>
#include "socks_session.hpp"

using namespace std;

io_service global_io_service;

Server::Server(unsigned short port) :
    acceptor_(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
    socket_(global_io_service)
{
    do_accept();
}

void Server::do_accept()
{
    acceptor_.async_accept(
        socket_,
        [this](boost::system::error_code ec) {
            if (!ec) {
                make_shared<Session>(move(socket_))->start();
            } else {
                cerr << ec.message() << endl;
            }
            do_accept();
        }
    );
}

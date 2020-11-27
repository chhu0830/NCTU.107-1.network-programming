#include "http_server.hpp"
#include "http_session.hpp"

io_service global_io_service;

Server::Server(short port) :
    acceptor_(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
    socket_(global_io_service)
    {
        do_accept();
    }

void Server::do_accept()
{
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        if (!ec) {
            make_shared<Session>(move(socket_))->start();
        }
        do_accept();
    });
}

#include "http_server.hpp"
#include "http_session.hpp"

io_service global_io_service;

Server::Server(short port):
    _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
    _socket(global_io_service) {
        do_accept();
    }

void Server::do_accept() {
    _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
        if (!ec) {
            make_shared<Session>(move(_socket))->start();
        }
        do_accept();
    });
}

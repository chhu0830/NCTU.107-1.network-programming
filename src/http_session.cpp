#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "http_server.hpp"
#include "http_session.hpp"

extern io_service global_io_service;

Session::Session(ip::tcp::socket socket) : _socket(move(socket)) {}

void Session::start() {
    do_read();
}

void Session::do_read() {
    auto self(shared_from_this());
    _socket.async_read_some(
        buffer(_data, MAX_LENGTH),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                cgi();
            }
        }
    );
}

void Session::cgi() {
    _request = Request(string(_data.data()));

    int pid;
    global_io_service.notify_fork(io_service::fork_prepare);
    if ((pid = fork()) < 0) {
        perror("Error");
    } else if (pid == 0) {
        global_io_service.notify_fork(io_service::fork_child);

        vector<char*> argv({(char*)_request.script_filename().c_str(), NULL});

        setenviron();

        dup2(_socket.native_handle(), 0);
        dup2(_socket.native_handle(), 1);
        dup2(_socket.native_handle(), 2);
        cout << _request.server_protocol() << " 200 OK" << endl;
        if (execvp(argv.front(), argv.data()) < 0) {
            perror("Error");
            exit(1);
        }
    } else {
        global_io_service.notify_fork(io_service::fork_parent);
        _socket.close();
    }
}

void Session::setenviron() {
    environ = NULL;
    
    for (const auto i : _request.headers()) {
        string variable = string("HTTP_") + boost::to_upper_copy<string>(boost::replace_all_copy(i.first, "-", "_"));
        setenv(variable.c_str(), i.second.c_str(), 1);
    }

    setenv("REQUEST_METHOD", _request.request_method().c_str(), 1);
    setenv("REQUEST_URI", _request.request_uri().c_str(), 1);
    setenv("QUERY_STRING", _request.query_string().c_str(), 1);
    setenv("SERVER_PROTOCOL", _request.server_protocol().c_str(), 1);
    setenv("SCRIPT_NAME", _request.script_name().c_str(), 1);
    setenv("SCRIPT_FILENAME", _request.script_filename().c_str(), 1);
    
    setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string().c_str(), 1);
    setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()).c_str(), 1);
    setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string().c_str(), 1);
    setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()).c_str(), 1);
}

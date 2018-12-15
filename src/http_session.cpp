#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "http_server.hpp"
#include "http_session.hpp"

extern io_service global_io_service;

Session::Session(ip::tcp::socket socket) : socket_(move(socket)) {}

void Session::start()
{
    do_read();
}

void Session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(
        buffer(data_, MAX_LENGTH),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                cgi();
            }
        }
    );
}

void Session::cgi()
{
    auto self(shared_from_this());
    request_ = Request(data_.data());

    int pid;
    global_io_service.notify_fork(io_service::fork_prepare);
    if ((pid = fork()) < 0) {
        perror("Error");
    } else if (pid == 0) {
        global_io_service.notify_fork(io_service::fork_child);

        vector<char*> argv({(char*)request_.script_filename().c_str(), NULL});

        setenviron();

        dup2(socket_.native_handle(), STDIN_FILENO);
        dup2(socket_.native_handle(), STDOUT_FILENO);
        // dup2(socket_.native_handle(), STDERR_FILENO);
        // cout << request_.server_protocol() << " 200 OK" << endl;
        // cout << "HTTP/1.1 200 OK" << endl;
        socket_.async_write_some(
            buffer("HTTP/1.1 200 OK"),
            [self](boost::system::error_code, size_t) {}
        );
        if (execvp(argv.front(), argv.data()) < 0) {
            perror("Error");
            exit(1);
        }
    } else {
        global_io_service.notify_fork(io_service::fork_parent);
        socket_.close();
    }
}

void Session::setenviron()
{
    environ = NULL;
    
    for (const auto i : request_.headers()) {
        string variable = "HTTP_" + boost::to_upper_copy<string>(boost::replace_all_copy(i.first, "-", "_"));
        setenv(variable.c_str(), i.second.c_str(), 1);
    }

    setenv("REQUEST_METHOD", request_.request_method().c_str(), 1);
    setenv("REQUEST_URI", request_.request_uri().c_str(), 1);
    setenv("QUERY_STRING", request_.query_string().c_str(), 1);
    setenv("SERVER_PROTOCOL", request_.server_protocol().c_str(), 1);
    setenv("SCRIPT_NAME", request_.script_name().c_str(), 1);
    setenv("SCRIPT_FILENAME", request_.script_filename().c_str(), 1);
    
    setenv("SERVER_ADDR", socket_.local_endpoint().address().to_string().c_str(), 1);
    setenv("SERVER_PORT", to_string(socket_.local_endpoint().port()).c_str(), 1);
    setenv("REMOTE_ADDR", socket_.remote_endpoint().address().to_string().c_str(), 1);
    setenv("REMOTE_PORT", to_string(socket_.remote_endpoint().port()).c_str(), 1);
}

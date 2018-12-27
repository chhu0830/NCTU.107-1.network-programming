#include "socks_session.hpp"
#include <iostream>
#include <fstream>
#include <regex>
#include "socks_server.hpp"

extern io_service global_io_service;

Session::Session(ip::tcp::socket socket) :
    src_socket_(move(socket)),
    dst_socket_(global_io_service),
    resolver_(global_io_service),
    acceptor_(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), 0)) {}

void Session::start()
{
    read_config();
    read_request();
}

void Session::read_config()
{
    ifstream fin("socks.conf", ifstream::in);
    string s;
    smatch m;

    while (getline(fin, s)) {
        if (regex_match(s, m, regex("([a-zA-Z]+) ([bc]) ([*0-9]{1,3})\\.([*0-9]{1,3})\\.([*0-9]{1,3})\\.([*0-9]{1,3}).*"))) {
            if (m.str(1) == "permit") {
                if (m.str(2) == "c") {
                    permits_[0].push_back({m.str(3), m.str(4), m.str(5), m.str(6)});
                } else if (m.str(2) == "b") {
                    permits_[1].push_back({m.str(3), m.str(4), m.str(5), m.str(6)});
                }
            }
        }
    }
}

void Session::read_request()
{
    auto self(shared_from_this());
    src_socket_.async_read_some(
        request_.to_buffers(),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                read_userid();
            } else {
                cerr << "read_request: " << ec.message() << endl;
            }
        }
    );
}

void Session::read_userid()
{
    auto self(shared_from_this());
    async_read_until(
        src_socket_,
        request_.userid,
        '\x00',
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                if (permit(request_.cd, request_.addr)) {
                    if (request_.cd == 1) {
                        do_resolve();
                    } else if (request_.cd == 2) {
                        do_accept();
                        write_reply(90, 2);
                    }
                } else {
                    write_reply(91, request_.cd);
                }
            } else {
                cerr << "read_userid: " << ec.message() << endl;
            }
        }
    );
}

bool Session::permit(int mode, ip::address_v4::bytes_type &addr)
{
    for (auto &i : permits_[(mode == 1 ? 0 : 1)]) {
        bool flag = true;
        for (int j = 0; flag && j < 4; j++) {
            flag = (i[j] == "*" || i[j] == to_string(addr[j]));
        }
        if (flag) return true;
    }

    return false;
}

void Session::do_accept()
{
    auto self(shared_from_this());
    acceptor_.async_accept(
        dst_socket_,
        [this, self](boost::system::error_code ec) {
            if (!ec) {
                if (ip::address_v4(request_.addr) == dst_socket_.remote_endpoint().address()) {
                    write_reply(90);
                } else {
                    write_reply(91);
                }
            } else {
                cerr << "do_accept: " << ec.message() << endl;
            }
        }
    );
}

void Session::do_resolve()
{
    auto self(shared_from_this());
    resolver_.async_resolve(
        ip::tcp::resolver::query(ip::address_v4(request_.addr).to_string(), to_string(ntohs(request_.port))),
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                cerr << "do_resolve: " << ec.message() << endl;
            }
        }
    );
}

void Session::do_connect(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());
    async_connect(
        dst_socket_,
        it,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
            if (!ec) {
                write_reply(90);
            } else {
                cerr << "do_connect: " << ec.message() << endl;
                write_reply(91);
            }

        }
    );
}

void Session::write_reply(uint8_t cd, uint8_t mode)
{
    auto self(shared_from_this());

    if (mode == 1) {
        reply_ = Reply(0, cd, request_.port, request_.addr);
    } else if (mode == 2) {
        reply_ = Reply(0, cd, htons(acceptor_.local_endpoint().port()), acceptor_.local_endpoint().address().to_v4().to_bytes());
    }

    show_info();

    async_write(
        src_socket_,
        reply_.to_buffers(),
        [this, self, mode](boost::system::error_code ec, size_t) {
            if (!ec) {
                if (reply_.accept() && mode == 1) {
                    do_read(0);
                    do_read(1);
                }
            } else {
                cerr << "write_reply: " << ec.message() << endl;
            }
        }
    );
}

void Session::show_info()
{
    if (src_socket_.is_open() == false) return;
    cout << "================================================" << endl;
    cout << "<S_IP>\t\t: " << src_socket_.remote_endpoint().address() << endl;
    cout << "<S_PORT>\t: " << src_socket_.remote_endpoint().port() << endl;
    cout << "<D_IP>\t\t: " << ip::address_v4(request_.addr) << endl;
    cout << "<D_PORT>\t: " << ntohs(request_.port) << endl;
    cout << "<Command>\t: " << (request_.cd == 1 ? "CONNECT" : "BIND") << endl;
    cout << "<Reply>\t\t: " << (reply_.cd == 90 ? "ACCEPT" : "REJECT") << endl;
    cout << "================================================" << endl << endl;
}

void Session::do_read(bool target)
{
    auto self(shared_from_this());

    ip::tcp::socket &socket = (target == 0 ? src_socket_ : dst_socket_);
    array<char, MAX_BUF_LENGTH> &buf = (target == 0 ? src_buffer_ : dst_buffer_);

    socket.async_read_some(
        buffer(buf),
        [this, self, target](boost::system::error_code ec, size_t length) {
            if (!ec) {
                do_write(!target, length);
            } else {
                cerr << "do_read(" << target << "): " << ec.message() << endl;
                target == 0 ? dst_socket_.close() : src_socket_.close();
            }
        }
    );
}

void Session::do_write(bool target, size_t length)
{
    auto self(shared_from_this());

    ip::tcp::socket &socket = (target == 0 ? src_socket_ : dst_socket_);
    array<char, MAX_BUF_LENGTH> &buf = (target == 0 ? dst_buffer_ : src_buffer_);

    async_write(
        socket,
        buffer(buf, length),
        [this, self, target](boost::system::error_code ec, size_t) {
            if (!ec) {
                do_read(!target);
            } else {
                cerr << "do_write: " << ec.message() << endl;
            }
        }
    );
}

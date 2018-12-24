#include "socks_session.hpp"
#include <iostream>
#include "socks_server.hpp"

extern io_service global_io_service;

/*========== Request ==========*/
Request::Request() {}

Request::Request(uint8_t vn, uint8_t cd, uint16_t port, array<uint8_t, 4> addr) :
    vn_(vn), cd_(cd), port_(port), addr_(addr) {}

vector<mutable_buffer> Request::to_buffers()
{
    return {
        buffer(&vn_, sizeof(vn_)),
        buffer(&cd_, sizeof(cd_)),
        buffer(&port_, sizeof(port_)),
        buffer(addr_)
    };
}

void Request::show()
{
    cerr << "====================" << endl;
    cerr << "VN:\t" << unsigned(vn()) << endl;
    cerr << "CD:\t" << unsigned(cd()) << endl;
    cerr << "PORT:\t" << port() << endl;
    cerr << "IP:\t" << addr() << endl;
    cerr << "ID:\t" << userid() << endl;
    cerr << "====================" << endl << endl;
}

mutable_buffer Request::userid_to_buffer()
{
    return buffer(&userid_, MAX_USERID_LENGTH);
}

uint8_t Request::vn()
{
    return vn_;
}

uint8_t Request::cd()
{
    return cd_;
}

uint16_t Request::port()
{
    return ntohs(port_);
}

string Request::addr()
{
    return to_string(addr_[0]) + "." + to_string(addr_[1]) + "." + to_string(addr_[2]) + "." + to_string(addr_[3]);
}

string Request::userid()
{
    return string(userid_.data());
}

bool Request::accept()
{
    return (cd_ == 90);
}

void Request::vn(uint8_t version)
{
    vn_ = version;
}

void Request::cd(uint8_t cd)
{
    cd_ = cd;
}

void Request::port(uint16_t port)
{
    port_ = port;
}

void Request::addr(array<uint8_t, 4> addr)
{
    addr_ = addr;
}

/*========== Session ==========*/
Session::Session(ip::tcp::socket socket) :
    src_socket_(move(socket)),
    dst_socket_(global_io_service),
    resolver_(global_io_service),
    acceptor_(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), 0)),
    flag(true) {}

void Session::start()
{
    read_request();
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
    src_socket_.async_read_some(
        request_.userid_to_buffer(),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                if (request_.cd() == 1) {
                    do_resolve();
                } else {
                    do_accept();
                    reply_ = Request(0, 90, htons(acceptor_.local_endpoint().port()), {0, 0, 0, 0});

                    async_write(
                        src_socket_,
                        reply_.to_buffers(),
                        [this, self](boost::system::error_code ec, size_t) {
                            if (ec) {
                                cerr << "read_userid(async_write): " << ec.message() << endl;
                            }
                        }
                    );
                }
            } else {
                cerr << "read_userid: " << ec.message() << endl;
            }
        }
    );
}

void Session::do_accept()
{
    auto self(shared_from_this());
    acceptor_.async_accept(
        dst_socket_,
        [this, self](boost::system::error_code ec) {
            if (!ec) {
                if (request_.addr() == dst_socket_.remote_endpoint().address().to_string()) {
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
        ip::tcp::resolver::query(request_.addr(), to_string(request_.port())),
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

void Session::write_reply(uint8_t cd)
{
    auto self(shared_from_this());

    reply_ = request_;
    reply_.vn(0);
    reply_.cd(cd);

    async_write(
        src_socket_,
        reply_.to_buffers(),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                do_read(0);
                do_read(1);
            } else {
                cerr << "write_reply: " << ec.message() << endl;
            }
        }
    );
}

void Session::show_info()
{
    cout << "================================================================" << endl;
    cout << "<S_IP>\t\t: " << src_socket_.remote_endpoint().address() << endl;
    cout << "<S_PORT>\t: " << src_socket_.remote_endpoint().port() << endl;
    cout << "<D_IP>\t\t: " << request_.addr() << endl;
    cout << "<D_PORT>\t: " << request_.port() << endl;
    cout << "<Command>\t: " << (request_.cd() == 1 ? "CONNECT" : "BIND") << endl;
    cout << "<Reply>\t\t: " << (reply_.cd() == 90 ? "ACCEPT" : "REJECT") << endl;
    cout << "<Content>\t: " << "'" << string(src_buffer_.data(), 20) << "' | '" << string(dst_buffer_.data(), 20) << "'" << endl;;
    cout << "================================================================" << endl << endl;
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
                if (flag) {
                    show_info();
                    flag = false;
                }
                do_write(!target, length);
            } else {
                cerr << "do_read: " << ec.message() << endl;
                src_socket_.close();
                dst_socket_.close();
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

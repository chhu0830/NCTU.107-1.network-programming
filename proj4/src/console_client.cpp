#include "console_client.hpp"
#include <iostream>
#include "console_session.hpp"

extern io_service global_io_service;

Client::Client(shared_ptr<Session> session, const string id) :
    session_(session),
    resolver_(global_io_service),
    socket_(global_io_service),
	id_(id),
    host_(session->target(id).host()),
    port_(session->target(id).port()),
    socks_host_(session->target(id).socks_host()),
    socks_port_(session->target(id).socks_port()),
    flag_(false),
    fin_("test_case/" + session->target(id).file(), ifstream::in) {}

void Client::start()
{
    auto self(shared_from_this());
    ip::tcp::resolver::query query = (socks_server_valid() ?
                                      ip::tcp::resolver::query(socks_host_, socks_port_) :
                                      ip::tcp::resolver::query(host_, port_));

    resolver_.async_resolve(
        query,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                session_->html_addcontent(id_, ec.message());
            }
        }
    );
}

bool Client::socks_server_valid()
{
    return (socks_host_.length() && socks_port_.length());
}

void Client::do_connect(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());

    async_connect(
        socket_,
        it,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
            if (!ec) {
                if (socks_server_valid()) {
                    resolver_.async_resolve(
                        ip::tcp::resolver::query(host_, port_),
                        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
                            if (!ec) {
                                do_socks_request(it);
                            } else {
                                session_->html_addcontent(id_, ec.message());
                            }
                        }
                    );
                } else {
                    do_read();
                }
            } else {
                session_->html_addcontent(id_, ec.message());
            }
        }
    );
}

void Client::do_socks_request(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());
    request_ = Request(4, 1, htons(stoi(port_)), it->endpoint().address().to_v4().to_bytes());

    async_write(
        socket_,
        request_.to_buffers(),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                do_socks_reply();
            } else {
                session_->html_addcontent(id_, ec.message());
            }
        }
    );
}

void Client::do_socks_reply()
{
    auto self(shared_from_this());

    socket_.async_read_some(
        reply_.to_buffers(),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                if (reply_.accept()) {
                    do_read();
                }
            } else {
                session_->html_addcontent(id_, ec.message());
            }
        }
    );
}

void Client::do_read()
{
    auto self(shared_from_this());

    socket_.async_read_some(
        buffer(recvmsg_, MAX_LENGTH),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                string received(recvmsg_.data(), length);
                session_->html_addcontent(id_, session_->html_escape(received));

                for (auto &ch : received) {
                    if (flag_ && (ch == ' ')) {
                        flag_ = false;
                        do_write();
                    }
                    flag_ = (ch == '%');
                }
                do_read();
            }
        }
    );
}

void Client::do_write()
{
    auto self(shared_from_this());
    getline(fin_, sendmsg_);
    sendmsg_ += "\n";
    
    async_write(
        socket_,
        buffer(sendmsg_.c_str(), sendmsg_.length()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                session_->html_addcontent(id_, "<b>" + session_->html_escape(sendmsg_) + "</b>");
            }
        }
    );
}

#include "console_client.hpp"
#include "console_session.hpp"

extern io_service global_io_service;

Client::Client(shared_ptr<Session> session, const string id) :
    session_(session),
    resolver_(global_io_service),
    socket_(global_io_service),
	id_(id),
    host_(session->target(id).host()),
    port_(session->target(id).port()),
    flag_(false),
    fin_("test_case/" + session->target(id).file(), ifstream::in) {}

void Client::start()
{
    do_resolve();
}

void Client::do_resolve()
{
    auto self(shared_from_this());
    resolver_.async_resolve(
        ip::tcp::resolver::query(host_, port_),
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                session_->html_addcontent(id_, ec.message());
            }
        }
    );
}

void Client::do_connect(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());

    async_connect(
        socket_,
        it,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
            if (!ec) {
                do_read();
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
        buffer(recvmsg_, MAX_LENGTH - 1),
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
    
    socket_.async_send(
        buffer(sendmsg_.c_str(), sendmsg_.length()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                session_->html_addcontent(id_, "<b>" + session_->html_escape(sendmsg_) + "</b>");
            }
        }
    );
}

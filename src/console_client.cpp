#include "console_session.hpp"
#include "console_client.hpp"

io_service global_io_service;

Client::Client(Session &session, const string id) :
    _session(session),
    _resolver(global_io_service),
    _socket(global_io_service),
	_id(id),
    _host(session.target(id).host()),
    _port(session.target(id).port()),
    _fin(string("test_case/") + session.target(id).file(), ifstream::in) {}

void Client::start()
{
    do_resolve();
}

void Client::do_resolve()
{
    auto self(shared_from_this());
    _resolver.async_resolve(
        ip::tcp::resolver::query(_host, _port),
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                _session.html_addcontent(_id, ec.message());
            }
        }
    );
}

void Client::do_connect(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());

    async_connect(
        _socket,
        it,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
            if (!ec) {
                do_read();
            } else {
                _session.html_addcontent(_id, ec.message());
            }
        }
    );
}

void Client::do_read()
{
    auto self(shared_from_this());

    fill(_recvmsg.begin(), _recvmsg.end(), '\0');
    _socket.async_read_some(
        buffer(_recvmsg, MAX_LENGTH-1),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                _session.html_addcontent(_id, _session.html_plaintext(string(_recvmsg.data())));
                if ((_flag && string(_recvmsg.data()).front() == ' ') || string(_recvmsg.data()).find("% ") != string::npos) {
                    do_write();
                } else {
                    _flag = (_recvmsg[length-1] == '%');
                    do_read();
                }
            }
        }
    );
}

void Client::do_write()
{
    auto self(shared_from_this());
    getline(_fin, _sendmsg);
    _sendmsg += "\n";
    
    _socket.async_send(
        buffer(_sendmsg.c_str(), _sendmsg.length()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                _session.html_addcontent(_id, "<b>" + _session.html_plaintext(string(_sendmsg.data())) + "</b>");
                do_read();
            }
        }
    );
}

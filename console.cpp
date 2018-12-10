#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>
#include <fstream>

using namespace std;
using namespace boost::asio;

io_service global_io_service;

class Session {
    public:
        Session(string query);
        void response_html(string raw);
};

class Client : public enable_shared_from_this<Client> {
    private:
        enum { MAX_LENGTH = 1024 };
        Session &_session;
        ip::tcp::resolver _resolver;
        ip::tcp::socket _socket;
        string _host, _port;
        ifstream _fin;
        array<char, MAX_LENGTH> _recvmsg;
        string _sendmsg;
        bool _flag;

    public:
        Client(Session &session, const string host, const string port, const string filename);
        void start();

    private:
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read();
        void do_write();
};

Client::Client(Session &session, const string host, const string port, const string filename) :
    _session(session),
    _resolver(global_io_service),
    _socket(global_io_service),
    _host(host),
    _port(port),
    _fin(string("test_case/") + filename, ifstream::in) {}

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
            }
        }
    );
}

void Client::do_read()
{
    auto self(shared_from_this());

    fill(_recvmsg.begin(), _recvmsg.end(), '\0');
    _socket.async_read_some(
        buffer(_recvmsg, MAX_LENGTH),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                // cout << _recvmsg.data();
                _session.response_html(string(_recvmsg.data()));
                if ((_flag && string(_recvmsg.data()).front() == ' ') || string(_recvmsg.data()).find("% ") != string::npos) {
                    do_write();
                } else {
                    _flag = (string(_recvmsg.data()).back() == '%');
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
                // cout << _sendmsg;
                _session.response_html(string(_sendmsg.data()));
                do_read();
            }
        }
    );
}

Session::Session(string query)
{
    const string QUERYREX = "h[0-9]=([a-zA-Z0-9.]*)&p[0-9]=([0-9]*)&f[0-9]=([a-zA-Z0-9.]*)";
    for (smatch m; regex_search(query, m, regex(QUERYREX)); query = m.suffix().str()) {
        if (m.str(1).length() && m.str(2).length() && m.str(3).length()) {
            make_shared<Client>(*this, m.str(1), m.str(2), m.str(3))->start();
        }
    }
}

void Session::response_html(string raw)
{
    // boost::replace_all(raw, "\r\n", "<br />");
    raw = regex_replace(raw, std::regex("<"), "&lt;");
    raw = regex_replace(raw, std::regex(">"), "&gt;");
    raw = regex_replace(raw, std::regex("[\r\n]+"), "&NewLine;");
    cout << raw;
}

int main(int argc, const char *argv[])
{
    cout << "Content-type: text/html" << endl << endl;

    try {
        Session session(string(getenv("QUERY_STRING")));
        global_io_service.run();
    } catch (exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}

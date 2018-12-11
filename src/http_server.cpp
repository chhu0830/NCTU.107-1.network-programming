#include <iostream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>
#include <vector>
#include <sstream>

#define PCHAR "(?:[a-zA-Z0-9-._~!$&'/*+,;=:@]|%[a-fA-F0-9]{2})"
#define TOKEN "(?:[-!#$%&'*+.^_`|~a-zA-Z0-9])+"
#define VCHAR "(?:[\x21-\x7e])"
#define OWS "(?:[ \t]*)"
#define CONTENT "(?:" VCHAR "(?:[ \t]+" VCHAR ")?)"

extern char **environ;

using namespace std;
using namespace boost::asio;

io_service global_io_service;

class Request {
    private:
        string REQUEST_METHOD;
        string REQUEST_URI;
        string REQUEST_PATH;
        string QUERY_STRING;
        string SERVER_PROTOCOL;
        map<string, string> HEADERS;

    public:
        Request() {}

        Request(string request)
        {
            stringstream ss(request);
            string str;
            smatch m;

            getline(ss, str);
            if (regex_match(str, m, regex("(" TOKEN ") (((?:/" PCHAR "*){1,})[?]?((?:" PCHAR "|[/?])*)) (HTTP/[0-9]\\.[0-9])\r"))) {
                REQUEST_METHOD = m.str(1);
                REQUEST_URI = m.str(2);
                REQUEST_PATH = m.str(3);
                QUERY_STRING = m.str(4);
                SERVER_PROTOCOL = m.str(5);
            }

            while (getline(ss, str)) {
                if (regex_match(str, m, regex("(" TOKEN "):" OWS "((?:" CONTENT ")*)" OWS "\r"))) {
                    HEADERS[m.str(1)] = m.str(2);
                }
            }
        }

        string request_method() { return REQUEST_METHOD; }
        string request_uri() { return REQUEST_URI; }
        string request_path() { return REQUEST_PATH; }
        string query_string() { return QUERY_STRING; }
        string server_protocol() { return SERVER_PROTOCOL; }
        map<string, string> headers() { return HEADERS; }
};

class Session : public enable_shared_from_this<Session> {
    private:
        enum { MAX_LENGTH = 1024 };
        ip::tcp::socket _socket;
        array<char, MAX_LENGTH> _data;
        Request _request;
    
    public:
        Session(ip::tcp::socket socket) : _socket(move(socket)) {}

        void start() {
            do_read();
        }

    private:
        void do_read() {
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

        void cgi() {
            _request = Request(string(_data.data()));

            int pid;
            global_io_service.notify_fork(io_service::fork_prepare);
            if ((pid = fork()) < 0) {
                perror("Error");
            } else if (pid == 0) {
                global_io_service.notify_fork(io_service::fork_child);

                string filename = boost::filesystem::current_path().string() + _request.request_path();
                vector<char*> argv({(char*)filename.c_str(), NULL});

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
        
        void setenviron() {
            environ = NULL;
            
            for (const auto i : _request.headers()) {
                string variable = string("HTTP_") + boost::to_upper_copy<string>(boost::replace_all_copy(i.first, "-", "_"));
                setenv(variable.c_str(), i.second.c_str(), 1);
            }

            setenv("REQUEST_METHOD", _request.request_method().c_str(), 1);
            setenv("REQUEST_URI", _request.request_uri().c_str(), 1);
            setenv("QUERY_STRING", _request.query_string().c_str(), 1);
            setenv("SERVER_PROTOCOL", _request.server_protocol().c_str(), 1);
            
            setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string().c_str(), 1);
            setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()).c_str(), 1);
            setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string().c_str(), 1);
            setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()).c_str(), 1);
        }
};

class Server {
    private:
        ip::tcp::acceptor _acceptor;
        ip::tcp::socket _socket;

    public:
        Server(short port):
            _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
            _socket(global_io_service) {
                do_accept();
            }

    private:
        void do_accept() {
            _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
                if (!ec) {
                    make_shared<Session>(move(_socket))->start();
                }
                do_accept();
            });
        }
};

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " [port]" << endl;
        return -1;
    }

    try {
        short port = atoi(argv[1]);
        Server server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}

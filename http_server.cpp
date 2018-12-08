#include <iostream>
#include <boost/asio.hpp>
#include <regex>
#include <vector>


using namespace std;
using namespace boost::asio;

io_service global_io_service;

struct Header {
    string request_method;
    string request_uri;
    string query_string;
    string server_protocol;
    string http_host;
    string server_addr;
    string remote_addr;
    short server_port;
    short remote_port;
};

class Session : public enable_shared_from_this<Session> {
    private:
        enum { MAX_LENGTH = 1024 };
        ip::tcp::socket _socket;
        array<char, MAX_LENGTH> _data;
    
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
                [this, self](boost::system::error_code ec, size_t length) {
                    if (!ec) {
                        cgi();
                    }
                }
            );
        }

        void cgi() {
            parse();
        }
        
        void parse() {
            // auto env = boost::this_process::environment();
            string header(_data.data());
            smatch m;

            cout << header;

            regex_search(header, m, regex("^[\\w]+"));
            cout << "request_method: " << m.str(0) << endl;
            setenv("REQUEST_METHOD", m.str(0).c_str(), 1);
            // env["REQUEST_METHOD"] = m.str(0).c_str();

            regex_search(header, m, regex("[\\w\\d]+\\.cgi"));
            cout << "request_uri: " << m.str(0) << endl;
            setenv("REQUEST_URI", m.str(0).c_str(), 1);

            regex_search(header, m, regex("([\\w\\d]+=[\\w\\d]&*)+"));
            cout << "query_string: " << m.str(0) << endl;
            setenv("QUERY_STRING", m.str(0).c_str(), 1);
            
            regex_search(header, m, regex("[\\w]+/[\\d\\.]+"));
            cout << "server_protocol: " << m[0] << endl;
            setenv("SERVER_PROTOCOL", m.str(0).c_str(), 1);

            regex_search(header, m, regex("[\\d]+\\.[\\d]+\\.[\\d]+\\.[\\d]+:[\\d]+"));
            cout << "http_host: " << m.str(0) << endl;
            setenv("HTTP_HOST", m.str(0).c_str(), 1);

            cout << "server_addr: " << _socket.local_endpoint().address().to_string().c_str() << endl;
            setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string().c_str(), 1);

            cout << "server_port: " << _socket.local_endpoint().port() << endl;
            setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()).c_str(), 1);

            cout << "remote_addr: " << _socket.remote_endpoint().address() << endl;
            setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string().c_str(), 1);

            cout << "remote_port: " << _socket.remote_endpoint().port() << endl;
            setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()).c_str(), 1);

            // std::transform(env.begin(), env.end(), _env.begin(), std::mem_fun_ref(&std::string::c_str));
        }
};

class Server {
    private:
        ip::tcp::acceptor _acceptor;
        ip::tcp::socket _socket;

    public:
        Server(short port): _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
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

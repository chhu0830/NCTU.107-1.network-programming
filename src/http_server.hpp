#include <boost/asio.hpp>

using namespace boost::asio;

class Server {
    private:
        ip::tcp::acceptor _acceptor;
        ip::tcp::socket _socket;

    public:
        Server(short port);

    private:
        void do_accept();
};


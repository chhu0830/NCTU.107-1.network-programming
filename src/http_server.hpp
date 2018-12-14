#include <boost/asio.hpp>

using namespace boost::asio;

class Server {
    private:
        ip::tcp::acceptor acceptor_;
        ip::tcp::socket socket_;

    public:
        Server(short port);

    private:
        void do_accept();
};


#include "http_request.hpp"

using namespace std;
using namespace boost::asio;

class Session : public enable_shared_from_this<Session> {
    private:
        enum { MAX_LENGTH = 1024 };
        ip::tcp::socket socket_;
        array<char, MAX_LENGTH> data_;
        Request request_;
    
    public:
        Session(ip::tcp::socket socket);
        void start();

    private:
        void do_read();
        void cgi();
        void setenviron();
};

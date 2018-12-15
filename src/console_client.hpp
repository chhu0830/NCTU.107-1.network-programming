#include <fstream>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Session;

class Client : public enable_shared_from_this<Client> {
    private:
        enum { MAX_LENGTH = 1024 };
        shared_ptr<Session> session_;
        ip::tcp::resolver resolver_;
        ip::tcp::socket socket_;
        string id_, host_, port_;
        ifstream fin_;
        array<char, MAX_LENGTH> recvmsg_;
        string sendmsg_;
        bool flag_;

    public:
        Client(shared_ptr<Session> session, const string id);
        void start();

    private:
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read();
        void do_write();
};

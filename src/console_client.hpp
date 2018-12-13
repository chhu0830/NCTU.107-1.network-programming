#include <memory>
#include <boost/asio.hpp>
#include <fstream>

using namespace std;
using namespace boost::asio;

class Client : public enable_shared_from_this<Client>
{
    private:
        enum { MAX_LENGTH = 1024 };
        Session &_session;
        ip::tcp::resolver _resolver;
        ip::tcp::socket _socket;
        string _id, _host, _port;
        ifstream _fin;
        array<char, MAX_LENGTH> _recvmsg;
        string _sendmsg;
        bool _flag;

    public:
        Client(Session &session, const string id);
        void start();

    private:
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read();
        void do_write();
};

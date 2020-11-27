#include <fstream>
#include <functional>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

struct Request {
    uint8_t vn;
    uint8_t cd;
    uint16_t port;
    ip::address_v4::bytes_type addr;
    uint8_t null_byte;

    Request() {}

    Request(uint8_t vn, uint8_t cd, uint16_t port, ip::address_v4::bytes_type addr) :
        vn(vn), cd(cd), port(port), addr(addr), null_byte(0) {}

    vector<mutable_buffer> to_buffers()
    {
        return {
            buffer(&vn, sizeof(vn)),
            buffer(&cd, sizeof(cd)),
            buffer(&port, sizeof(port)),
            buffer(addr),
            buffer(&null_byte, sizeof(null_byte))
        };
    }
};

struct Reply {
    uint8_t vn;
    uint8_t cd;
    uint16_t port;
    array<uint8_t, 4> addr;

    Reply() {}

    vector<mutable_buffer> to_buffers()
    {
        return {
            buffer(&vn, sizeof(vn)),
            buffer(&cd, sizeof(cd)),
            buffer(&port, sizeof(port)),
            buffer(addr)
        };
    }

    bool accept()
    {
        return (cd == 90);
    }
};

class Session;

class Client : public enable_shared_from_this<Client> {
    private:
        enum { MAX_LENGTH = 1024 };
        shared_ptr<Session> session_;
        ip::tcp::resolver resolver_;
        ip::tcp::socket socket_;
        string id_, host_, port_;
        string socks_host_, socks_port_;
        Request request_;
        Reply reply_;
        bool flag_;
        ifstream fin_;
        array<char, MAX_LENGTH> recvmsg_;
        string sendmsg_;

    public:
        Client(shared_ptr<Session> session, const string id);
        void start();

    private:
        bool socks_server_valid();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_socks_request(ip::tcp::resolver::iterator it);
        void do_socks_reply();
        void do_read();
        void do_write();
};

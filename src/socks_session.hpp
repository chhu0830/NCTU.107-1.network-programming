#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

struct Request {
    uint8_t vn;
    uint8_t cd;
    uint16_t port;
    ip::address_v4::bytes_type addr;
    boost::asio::streambuf userid;

    Request() {}

    vector<mutable_buffer> to_buffers()
    {
        return {
            buffer(&vn, sizeof(vn)),
            buffer(&cd, sizeof(cd)),
            buffer(&port, sizeof(port)),
            buffer(addr)
        };
    }
};

struct Reply {
    uint8_t vn;
    uint8_t cd;
    uint16_t port;
    ip::address_v4::bytes_type addr;

    Reply() {}

    Reply(uint8_t vn, uint8_t cd, uint16_t port, ip::address_v4::bytes_type addr) :
        vn(vn), cd(cd), port(port), addr(addr) {}

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

class Session : public enable_shared_from_this<Session> {
    private:
        enum { MAX_BUF_LENGTH = 4096 };
        Request request_;
        Reply reply_;
        vector<array<string, 4>> permits_[2];
        ip::tcp::socket src_socket_, dst_socket_;
        ip::tcp::resolver resolver_;
        ip::tcp::acceptor acceptor_;
        array<char, MAX_BUF_LENGTH> src_buffer_, dst_buffer_;
        bool flag;

    public:
        Session(ip::tcp::socket socket);
        void start();

    private:
        void read_config();
        void read_request();
        void read_userid();
        void write_reply(uint8_t cd);
        void show_info();
        bool permit(int mode, ip::address_v4::bytes_type &addr);
        void do_accept();
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read(bool target);
        void do_write(bool target, size_t length);
};

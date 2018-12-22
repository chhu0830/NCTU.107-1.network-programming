#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Request {
    private:
        enum { MAX_USERID_LENGTH = 1024 };
        uint8_t vn_;
        uint8_t cd_;
        uint16_t port_;
        array<uint8_t, 4> addr_;
        uint8_t command_;
        uint8_t reply_;
        array<char, MAX_USERID_LENGTH> userid_;

    public:
        Request();
        vector<mutable_buffer> to_buffers();
        mutable_buffer userid_to_buffer();
        void show();
        uint8_t vn();
        uint8_t cd();
        uint8_t command();
        uint8_t reply();
        uint16_t port();
        string addr();
        string userid();
        bool accept();
        void vn(uint8_t version);
        void cd(uint8_t command);
        void command(uint8_t command);
        void reply(uint8_t reply);
        // uint8_t& addr(int i);
};

class Session : public enable_shared_from_this<Session> {
    private:
        enum { MAX_BUF_LENGTH = 1024 };
        Request request_;
        ip::tcp::socket src_socket_, dst_socket_;
        ip::tcp::resolver resolver_;
        array<char, MAX_BUF_LENGTH> src_buffer_, dst_buffer_;
        bool flag;

    public:
        Session(ip::tcp::socket socket);
        void start();

    private:
        void read_request();
        void read_userid();
        void write_reply(uint8_t command);
        void show_info();
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read(short target);
        void do_write(short target, size_t length);
};

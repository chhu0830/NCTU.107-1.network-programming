#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Target {
    private:
        string host_, port_, file_;
        string socks_host_, socks_port_;

    public:
        Target() {}
        bool valid();
        void host(string host);
        void port(string port);
        void file(string file);
        void socks_host(string socks_host);
        void socks_port(string socks_port);
        string& host();
        string& port();
        string& file();
        string& socks_host();
        string& socks_port();
};

class Session : public enable_shared_from_this<Session> {
    private:
        posix::stream_descriptor out_;
        map<string, Target> target_;
        string buf_;
        bool flag_;
        void html_response();

    public:
        Session(posix::stream_descriptor out, string query);
        void start();
        void html_console();
        void html_addcontent(string id, string content);
        string html_escape(string text);
        Target& target(string id);
};

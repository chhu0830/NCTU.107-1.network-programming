#include <map>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

class Target {
    private:
        string host_, port_, file_;

    public:
        Target() {}
        bool valid();
        void host(string host);
        void port(string port);
        void file(string file);
        string& host();
        string& port();
        string& file();
};

class Session {
    private:
        ip::tcp::socket socket_;
        map<string, Target> target_;

    public:
        Session(string query);
        void html_template();
        void html_addcontent(string id, string content);
        string html_plaintext(string text);
        Target& target(string id);
};

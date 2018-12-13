#include <map>

using namespace std;

class Target {
    private:
        string _host, _port, _file;

    public:
        Target() {}

        bool valid() {
            return _host.length() && _port.length() && _file.length();
        }

        void host(string host) { _host = host; }
        void port(string port) { _port = port; }
        void file(string file) { _file = file; }
        string& host() { return _host; }
        string& port() { return _port; }
        string& file() { return _file;}
};

class Session {
    private:
        map<string, Target> _target;

    public:
        Session(string query);
        void html_template();
        void html_addcontent(string id, string content);
        string html_plaintext(string text);
        Target& target(string id);
};

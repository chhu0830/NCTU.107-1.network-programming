#include <iostream>
#include <boost/asio.hpp>
#include <regex>
#include <fstream>

using namespace std;
using namespace boost::asio;

io_service global_io_service;

class Session {
    private:
        int _count;
    public:
        Session(string query);
        string plaintext(string raw);
        void template_html();
        void response_html(string id, string raw);
};

class Client : public enable_shared_from_this<Client> {
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
        Client(Session &session, const string id, const string host, const string port, const string filename);
        void start();

    private:
        void do_resolve();
        void do_connect(ip::tcp::resolver::iterator it);
        void do_read();
        void do_write();
};

Client::Client(Session &session, const string id, const string host, const string port, const string filename) :
    _session(session),
    _resolver(global_io_service),
    _socket(global_io_service),
	_id(id),
    _host(host),
    _port(port),
    _fin(string("test_case/") + filename, ifstream::in) {}

void Client::start()
{
    do_resolve();
}

void Client::do_resolve()
{
    auto self(shared_from_this());
    _resolver.async_resolve(
        ip::tcp::resolver::query(_host, _port),
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            }
        }
    );
}

void Client::do_connect(ip::tcp::resolver::iterator it)
{
    auto self(shared_from_this());

    async_connect(
        _socket,
        it,
        [this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
            if (!ec) {
                do_read();
            }
        }
    );
}

void Client::do_read()
{
    auto self(shared_from_this());

    fill(_recvmsg.begin(), _recvmsg.end(), '\0');
    _socket.async_read_some(
        buffer(_recvmsg, MAX_LENGTH-1),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                _session.response_html("s"+_id, _session.plaintext(string(_recvmsg.data())));
                if ((_flag && string(_recvmsg.data()).front() == ' ') || string(_recvmsg.data()).find("% ") != string::npos) {
                    do_write();
                } else {
                    _flag = (_recvmsg[length-1] == '%');
                    do_read();
                }
            }
        }
    );
}

void Client::do_write()
{
    auto self(shared_from_this());
    getline(_fin, _sendmsg);
    _sendmsg += "\n";
    
    _socket.async_send(
        buffer(_sendmsg.c_str(), _sendmsg.length()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                _session.response_html("s"+_id, "<b>"+_session.plaintext(string(_sendmsg.data()))+"</b>");
                do_read();
            }
        }
    );
}

Session::Session(string query) : _count(0)
{
    template_html();
    const string QUERYREX = "h([0-9])=([a-zA-Z0-9.]*)&p[0-9]=([0-9]*)&f[0-9]=([a-zA-Z0-9.]*)";
    for (smatch m; regex_search(query, m, regex(QUERYREX)); query = m.suffix().str()) {
        if (m.str(2).length() && m.str(3).length() && m.str(4).length()) {
            _count++;
            response_html("h"+m.str(1), m.str(2)+":"+m.str(3));
            make_shared<Client>(*this, m.str(1), m.str(2), m.str(3), m.str(4))->start();
        }
    }
}

void Session::template_html()
{
	cout <<
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
            "<meta charset=\"UTF-8\" />\n"
            "<title>NP Project 3 Console</title>\n"
            "<link\n"
                "rel=\"stylesheet\"\n"
                "href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\"\n"
                "integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\n"
                "crossorigin=\"anonymous\"\n"
            "/>\n"
            "<link\n"
                "href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
                "rel=\"stylesheet\"\n"
            "/>\n"
            "<link\n"
                "rel=\"icon\"\n"
                "type=\"image/png\"\n"
                "href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n"
            "/>\n"
            "<style>\n"
                "* {\n"
                    "font-family: 'Source Code Pro', monospace;\n"
                    "font-size: 1rem !important;\n"
                "}\n"
                "body {\n"
                    "background-color: #212529;\n"
                "}\n"
                "pre {\n"
                    "color: #cccccc;\n"
                "}\n"
                "b {\n"
                    "color: #ffffff;\n"
                "}\n"
            "</style>\n"
        "</head>\n"
        "<body>\n"
            "<table class=\"table table-dark table-bordered\">\n"
                "<thead>\n"
                    "<tr>\n"
                        "<th scope=\"col\" id=\"h0\"></th>\n"
                        "<th scope=\"col\" id=\"h1\"></th>\n"
                        "<th scope=\"col\" id=\"h2\"></th>\n"
                        "<th scope=\"col\" id=\"h3\"></th>\n"
                        "<th scope=\"col\" id=\"h4\"></th>\n"
                    "</tr>\n"
                "</thead>\n"
                "<tbody>\n"
                    "<tr>\n"
                        "<td><pre id=\"s0\" class=\"mb-0\"></pre></td>\n"
                        "<td><pre id=\"s1\" class=\"mb-0\"></pre></td>\n"
                        "<td><pre id=\"s2\" class=\"mb-0\"></pre></td>\n"
                        "<td><pre id=\"s3\" class=\"mb-0\"></pre></td>\n"
                        "<td><pre id=\"s4\" class=\"mb-0\"></pre></td>\n"
                    "</tr>\n"
                "</tbody>\n"
            "</table>\n"
        "</body>\n"
        "</html>\n";
}

string Session::plaintext(string raw)
{
    string out;
    for (auto &ch : raw) {
        if (ch == '<') {
            out += "&lt;";
        } else if (ch == '>') {
            out += "&gt;";
        } else if (ch == '&') {
            out += "&amp;";
        } else if (ch == '"') {
            out += "&quot;";
        } else if (ch == '\'') {
            out += "&#039;";
        } else if (ch == '\n') {
            out += "&NewLine;";
        } else if (ch != '\r') {
            out += ch;
        }
    }

    return out;
}

void Session::response_html(string id, string raw)
{
    cout << "<script>" << "document.getElementById('" + id + "').innerHTML += '" + raw + "'</script>" << endl;
}

int main(int argc, const char *argv[])
{
    cout << "Content-type: text/html" << endl << endl;

    try {
        Session session(string(getenv("QUERY_STRING")));
        global_io_service.run();
    } catch (exception &e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}

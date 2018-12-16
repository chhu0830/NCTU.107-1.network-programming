#include <iostream>
#include <vector>
#include <sstream>
#define _REGEX_MAX_STACK_COUNT 200000
#include <regex>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#define TOKEN "(?:[-!#$%&'*+.^_`|~a-zA-Z0-9])+"
#define PCHAR "(?:[-._~!$&'/*+,;=:@a-zA-Z0-9]|%[a-fA-F0-9]{2})"
#define VCHAR "[\x21-\x7e]"
#define OWS "[ \t]*"

#define QUERY "(?:" PCHAR "|[/?])*"
#define ABSOLUTE_PATH "(?:/" PCHAR "*)+"
#define REQUEST_TARGET "(" ABSOLUTE_PATH ")(?:\\?(" QUERY "))?"
#define HTTP_VERSION "HTTP/[0-9]\\.[0-9]"
#define METHOD TOKEN
#define REQUEST_LINE "(" METHOD ") (" REQUEST_TARGET ") (" HTTP_VERSION ")\r"

#define FIELD_VALUE "(?:" VCHAR "(?:[ \t]+" VCHAR ")?)*"
#define FIELD_NAME TOKEN
#define HEADER_FIELD "(" FIELD_NAME "):" OWS "(" FIELD_VALUE ")" OWS "\r"

using namespace std;
using namespace boost::asio;

static const string PANEL(
"<!DOCTYPE html>"
"<html lang=\"en\">"
"  <head>"
"    <title>NP Project 3 Panel</title>"
"    <link"
"      rel=\"stylesheet\""
"      href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\""
"      integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\""
"      crossorigin=\"anonymous\""
"    />"
"    <link"
"      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""
"      rel=\"stylesheet\""
"    />"
"    <link"
"      rel=\"icon\""
"      type=\"image/png\""
"      href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\""
"    />"
"    <style>"
"      * {"
"        font-family: 'Source Code Pro', monospace;"
"      }"
"    </style>"
"  </head>"
"  <body class=\"bg-secondary pt-5\">"
"    <form action=\"console.cgi\" method=\"GET\">"
"      <table class=\"table mx-auto bg-light\" style=\"width: inherit\">"
"        <thead class=\"thead-dark\">"
"          <tr>"
"            <th scope=\"col\">#</th>"
"            <th scope=\"col\">Host</th>"
"            <th scope=\"col\">Port</th>"
"            <th scope=\"col\">Input File</th>"
"          </tr>"
"        </thead>"
"        <tbody>"
"          <tr>"
"            <th scope=\"row\" class=\"align-middle\">Session 1</th>"
"            <td>"
"              <div class=\"input-group\">"
"                <select name=\"h0\" class=\"custom-select\">"
"                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"npbsd1.cs.nctu.edu.tw\">npbsd1</option><option value=\"npbsd2.cs.nctu.edu.tw\">npbsd2</option><option value=\"npbsd3.cs.nctu.edu.tw\">npbsd3</option><option value=\"npbsd4.cs.nctu.edu.tw\">npbsd4</option><option value=\"npbsd5.cs.nctu.edu.tw\">npbsd5</option>"
"                </select>"
"                <div class=\"input-group-append\">"
"                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
"                </div>"
"              </div>"
"            </td>"
"            <td>"
"              <input name=\"p0\" type=\"text\" class=\"form-control\" size=\"5\" />"
"            </td>"
"            <td>"
"              <select name=\"f0\" class=\"custom-select\">"
"                <option></option>"
"                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option><option value=\"t6.txt\">t6.txt</option><option value=\"t7.txt\">t7.txt</option><option value=\"t8.txt\">t8.txt</option><option value=\"t9.txt\">t9.txt</option><option value=\"t10.txt\">t10.txt</option>"
"              </select>"
"            </td>"
"          </tr>"
"          <tr>"
"            <th scope=\"row\" class=\"align-middle\">Session 2</th>"
"            <td>"
"              <div class=\"input-group\">"
"                <select name=\"h1\" class=\"custom-select\">"
"                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"npbsd1.cs.nctu.edu.tw\">npbsd1</option><option value=\"npbsd2.cs.nctu.edu.tw\">npbsd2</option><option value=\"npbsd3.cs.nctu.edu.tw\">npbsd3</option><option value=\"npbsd4.cs.nctu.edu.tw\">npbsd4</option><option value=\"npbsd5.cs.nctu.edu.tw\">npbsd5</option>"
"                </select>"
"                <div class=\"input-group-append\">"
"                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
"                </div>"
"              </div>"
"            </td>"
"            <td>"
"              <input name=\"p1\" type=\"text\" class=\"form-control\" size=\"5\" />"
"            </td>"
"            <td>"
"              <select name=\"f1\" class=\"custom-select\">"
"                <option></option>"
"                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option><option value=\"t6.txt\">t6.txt</option><option value=\"t7.txt\">t7.txt</option><option value=\"t8.txt\">t8.txt</option><option value=\"t9.txt\">t9.txt</option><option value=\"t10.txt\">t10.txt</option>"
"              </select>"
"            </td>"
"          </tr>"
"          <tr>"
"            <th scope=\"row\" class=\"align-middle\">Session 3</th>"
"            <td>"
"              <div class=\"input-group\">"
"                <select name=\"h2\" class=\"custom-select\">"
"                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"npbsd1.cs.nctu.edu.tw\">npbsd1</option><option value=\"npbsd2.cs.nctu.edu.tw\">npbsd2</option><option value=\"npbsd3.cs.nctu.edu.tw\">npbsd3</option><option value=\"npbsd4.cs.nctu.edu.tw\">npbsd4</option><option value=\"npbsd5.cs.nctu.edu.tw\">npbsd5</option>"
"                </select>"
"                <div class=\"input-group-append\">"
"                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
"                </div>"
"              </div>"
"            </td>"
"            <td>"
"              <input name=\"p2\" type=\"text\" class=\"form-control\" size=\"5\" />"
"            </td>"
"            <td>"
"              <select name=\"f2\" class=\"custom-select\">"
"                <option></option>"
"                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option><option value=\"t6.txt\">t6.txt</option><option value=\"t7.txt\">t7.txt</option><option value=\"t8.txt\">t8.txt</option><option value=\"t9.txt\">t9.txt</option><option value=\"t10.txt\">t10.txt</option>"
"              </select>"
"            </td>"
"          </tr>"
"          <tr>"
"            <th scope=\"row\" class=\"align-middle\">Session 4</th>"
"            <td>"
"              <div class=\"input-group\">"
"                <select name=\"h3\" class=\"custom-select\">"
"                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"npbsd1.cs.nctu.edu.tw\">npbsd1</option><option value=\"npbsd2.cs.nctu.edu.tw\">npbsd2</option><option value=\"npbsd3.cs.nctu.edu.tw\">npbsd3</option><option value=\"npbsd4.cs.nctu.edu.tw\">npbsd4</option><option value=\"npbsd5.cs.nctu.edu.tw\">npbsd5</option>"
"                </select>"
"                <div class=\"input-group-append\">"
"                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
"                </div>"
"              </div>"
"            </td>"
"            <td>"
"              <input name=\"p3\" type=\"text\" class=\"form-control\" size=\"5\" />"
"            </td>"
"            <td>"
"              <select name=\"f3\" class=\"custom-select\">"
"                <option></option>"
"                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option><option value=\"t6.txt\">t6.txt</option><option value=\"t7.txt\">t7.txt</option><option value=\"t8.txt\">t8.txt</option><option value=\"t9.txt\">t9.txt</option><option value=\"t10.txt\">t10.txt</option>"
"              </select>"
"            </td>"
"          </tr>"
"          <tr>"
"            <th scope=\"row\" class=\"align-middle\">Session 5</th>"
"            <td>"
"              <div class=\"input-group\">"
"                <select name=\"h4\" class=\"custom-select\">"
"                  <option></option><option value=\"nplinux1.cs.nctu.edu.tw\">nplinux1</option><option value=\"nplinux2.cs.nctu.edu.tw\">nplinux2</option><option value=\"nplinux3.cs.nctu.edu.tw\">nplinux3</option><option value=\"nplinux4.cs.nctu.edu.tw\">nplinux4</option><option value=\"nplinux5.cs.nctu.edu.tw\">nplinux5</option><option value=\"npbsd1.cs.nctu.edu.tw\">npbsd1</option><option value=\"npbsd2.cs.nctu.edu.tw\">npbsd2</option><option value=\"npbsd3.cs.nctu.edu.tw\">npbsd3</option><option value=\"npbsd4.cs.nctu.edu.tw\">npbsd4</option><option value=\"npbsd5.cs.nctu.edu.tw\">npbsd5</option>"
"                </select>"
"                <div class=\"input-group-append\">"
"                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
"                </div>"
"              </div>"
"            </td>"
"            <td>"
"              <input name=\"p4\" type=\"text\" class=\"form-control\" size=\"5\" />"
"            </td>"
"            <td>"
"              <select name=\"f4\" class=\"custom-select\">"
"                <option></option>"
"                <option value=\"t1.txt\">t1.txt</option><option value=\"t2.txt\">t2.txt</option><option value=\"t3.txt\">t3.txt</option><option value=\"t4.txt\">t4.txt</option><option value=\"t5.txt\">t5.txt</option><option value=\"t6.txt\">t6.txt</option><option value=\"t7.txt\">t7.txt</option><option value=\"t8.txt\">t8.txt</option><option value=\"t9.txt\">t9.txt</option><option value=\"t10.txt\">t10.txt</option>"
"              </select>"
"            </td>"
"          </tr>"
"          <tr>"
"            <td colspan=\"3\"></td>"
"            <td>"
"              <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>"
"            </td>"
"          </tr>"
"        </tbody>"
"      </table>"
"    </form>"
"  </body>"
"</html>");

static const string CONSOLE(
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"  <head>\n"
"    <meta charset=\"UTF-8\" />\n"
"    <title>NP Project 3 Console</title>\n"
"    <link\n"
"      rel=\"stylesheet\"\n"
"      href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\"\n"
"      integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\n"
"      crossorigin=\"anonymous\"\n"
"    />\n"
"    <link\n"
"      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\n"
"      rel=\"stylesheet\"\n"
"    />\n"
"    <link\n"
"      rel=\"icon\"\n"
"      type=\"image/png\"\n"
"      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\n"
"    />\n"
"    <style>\n"
"      * {\n"
"        font-family: 'Source Code Pro', monospace;\n"
"        font-size: 1rem !important;\n"
"      }\n"
"      body {\n"
"        background-color: #212529;\n"
"      }\n"
"      pre {\n"
"        color: #cccccc;\n"
"      }\n"
"      b {\n"
"        color: #ffffff;\n"
"      }\n"
"    </style>\n"
"  </head>\n"
"  <body>\n"
"    <table class=\"table table-dark table-bordered\">\n"
"      <thead>\n"
"        <tr id=\"host\">\n"
"        </tr>\n"
"      </thead>\n"
"      <tbody>\n"
"        <tr id=\"session\">\n"
"        </tr>\n"
"      </tbody>\n"
"    </table>\n"
"  </body>\n"
"</html>\n");

class Server {
private:
	ip::tcp::acceptor acceptor_;
	ip::tcp::socket socket_;

public:
	Server(short port);

private:
	void do_accept();
};

class Request {
private:
	string request_method_;
	string request_uri_;
	string script_name_;
	string script_filename_;
	string query_string_;
	string server_protocol_;
	map<string, string> headers_;

public:
	Request();
	Request(string request);
	string& request_method();
	string& request_uri();
	string& script_name();
	string& script_filename();
	string& query_string();
	string& server_protocol();
	map<string, string>& headers();
};

class HTTP_Session : public enable_shared_from_this<HTTP_Session> {
private:
	
enum { MAX_LENGTH = 1024 };
	ip::tcp::socket socket_;
	array<char, MAX_LENGTH> data_;
	Request request_;

public:
	HTTP_Session(ip::tcp::socket socket);
	void start();

private:
	void do_read();
	void cgi();
};

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

class Console_Session : public enable_shared_from_this<Console_Session> {
private:
	ip::tcp::socket socket_;
	map<string, Target> target_;
	string buf_;
	bool flag_;
	void html_response();

public:
	Console_Session(ip::tcp::socket socket, string query);
	void start();
	void html_console();
	void html_addcontent(string id, string content);
	string html_escape(string text);
	Target& target(string id);

};

class Client : public enable_shared_from_this<Client> {
private:
	enum { MAX_LENGTH = 1024 };
	shared_ptr<Console_Session> session_;
	ip::tcp::resolver resolver_;
	ip::tcp::socket socket_;
	string id_, host_, port_;
	bool flag_;
	ifstream fin_;
	array<char, MAX_LENGTH> recvmsg_;
	string sendmsg_;

public:
	Client(shared_ptr<Console_Session> session, const string id);
	void start();

private:
	void do_resolve();
	void do_connect(ip::tcp::resolver::iterator it);
	void do_read();
	void do_write();
};

boost::asio::io_service global_io_service;

int main(int argc, const char *argv[])
{
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " [port]" << endl;
		exit(1);
	}

	try {
		short port = atoi(argv[1]);
		Server server(port);
		global_io_service.run();
	}
	catch (exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}

	return 0;
}

Server::Server(short port) :
	acceptor_(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
	socket_(global_io_service) {
		do_accept();
	}

void Server::do_accept() {
	acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
		if (!ec) {
			make_shared<HTTP_Session>(move(socket_))->start();
		}
		do_accept();
	});
}

HTTP_Session::HTTP_Session(ip::tcp::socket socket) : socket_(move(socket)) {}

void HTTP_Session::start()
{
	do_read();
}

void HTTP_Session::do_read()
{
	auto self(shared_from_this());
	socket_.async_read_some(
		buffer(data_, MAX_LENGTH),
		[this, self](boost::system::error_code ec, size_t) {
		if (!ec) {
			cgi();
		}
	}
	);
}

void HTTP_Session::cgi()
{
	auto self(shared_from_this());
	request_ = Request(data_.data());

	if (request_.script_name() == "/panel.cgi") {
		socket_.async_write_some(
			buffer("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" + PANEL),
			[this, self](boost::system::error_code ec, size_t) {
				if (!ec) {
					socket_.close();
				} else {
					cerr << ec.message() << endl;
				}
			}
		);
	} else if (request_.script_name() == "/console.cgi") {
		socket_.async_write_some(
			buffer("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" + CONSOLE),
			[this, self](boost::system::error_code ec, size_t) {
				if (!ec) {
					try {
						make_shared<Console_Session>(move(socket_), request_.query_string())->start();
					}
					catch (exception &e) {
						cerr << "Exception: " << e.what() << endl;
					}
				}
				else {
					cerr << ec.message() << endl;
				}
			}
		);
	}
}

Request::Request() {}

Request::Request(string request)
{
	stringstream ss(request);
	string str;
	smatch m;

	getline(ss, str);
	if (regex_match(str, m, regex(REQUEST_LINE))) {
		request_method_ = m.str(1);
		request_uri_ = m.str(2);
		script_name_ = m.str(3);
		script_filename_ = boost::filesystem::current_path().string() + script_name_;
		query_string_ = m.str(4);
		server_protocol_ = m.str(5);
	}

	while (getline(ss, str)) {
		if (regex_match(str, m, regex(HEADER_FIELD))) {
			headers_[m.str(1)] = m.str(2);
		}
	}
}

string& Request::request_method()
{
	return request_method_;
}

string& Request::request_uri()
{
	return request_uri_;
}

string& Request::script_name()
{
	return script_name_;
}

string& Request::script_filename()
{
	return script_filename_;
}

string& Request::query_string()
{
	return query_string_;
}

string& Request::server_protocol()
{
	return server_protocol_;
}

map<string, string>& Request::headers()
{
	return headers_;
}

bool Target::valid()
{
	return host_.length() && port_.length() && file_.length();
}

void Target::host(string host)
{
	host_ = host;
}

void Target::port(string port)
{
	port_ = port;
}

void Target::file(string file)
{
	file_ = file;
}

string& Target::host()
{
	return host_;
}

string& Target::port()
{
	return port_;
}

string& Target::file()
{
	return file_;
}

Console_Session::Console_Session(ip::tcp::socket socket, string query) : socket_(move(socket)), flag_(true)
{
	string s;
	stringstream ss(query);

	while (getline(ss, s, '&')) {
		int pos = s.find("=");
		string var = s.substr(0, pos);
		string val = s.substr(pos + 1);

		if (var[0] == 'h') {
			target_["s" + var.substr(1)].host(val);
		}
		else if (var[0] == 'p') {
			target_["s" + var.substr(1)].port(val);
		}
		else if (var[0] == 'f') {
			target_["s" + var.substr(1)].file(val);
		}
	}
}

void Console_Session::start()
{
	html_console();

	for (auto &i : target_) {
		if (i.second.valid()) {
			html_addcontent("host", "<th scope=\"col\">" + i.second.host() +
                                    ":" + html_escape(i.second.port()) +
                                    " [" + i.second.file() + "]" + "</th>");
			html_addcontent("session", "<td><pre id=\"" + i.first + "\" class=\"mb-0\"></pre></td>");

			make_shared<Client>(shared_from_this(), i.first)->start();
		}
	}

}

void Console_Session::html_console()
{
	auto self(shared_from_this());
	socket_.async_write_some(
		buffer(CONSOLE),
		[self](boost::system::error_code ec, size_t) {
			if (ec) {
				cerr << ec.message() << endl;
			}
		}
	);
}

void Console_Session::html_addcontent(string id, string content)
{
	buf_ += "<script>document.getElementById('" + id + "').innerHTML += '" + content + "'</script>";
	if (flag_) {
		flag_ = false;
		html_response();
	}
}

void Console_Session::html_response()
{
	auto self(shared_from_this());
	socket_.async_write_some(
		buffer(buf_),
		[this, self](boost::system::error_code ec, size_t length) {
		if (!ec) {
			buf_.erase(0, length);
			if (buf_.length()) {
				html_response();
			}
			else {
				flag_ = true;
			}
		}
		else {
			cerr << ec.message() << endl;
		}
	}
	);
}

string Console_Session::html_escape(string text)
{
	string out;
	for (auto &ch : text) {
		if (ch == '<') {
			out += "&lt;";
		}
		else if (ch == '>') {
			out += "&gt;";
		}
		else if (ch == '&') {
			out += "&amp;";
		}
		else if (ch == '"') {
			out += "&quot;";
		}
		else if (ch == '\'') {
			out += "&#039;";
		}
		else if (ch == '\\') {
			out += "&#092;";
		}
		else if (ch == '\n') {
			out += "&NewLine;";
		}
		else if (ch != '\r') {
			out += ch;
		}
	}

	return out;
}

Target& Console_Session::target(string id)
{
	return target_[id];
}

Client::Client(shared_ptr<Console_Session> session, const string id) :
	session_(session),
	resolver_(global_io_service),
	socket_(global_io_service),
	id_(id),
	host_(session->target(id).host()),
	port_(session->target(id).port()),
	flag_(false),
	fin_("test_case\\" + session->target(id).file(), ifstream::in) {}

void Client::start()
{
	do_resolve();
}

void Client::do_resolve()
{
	auto self(shared_from_this());
	resolver_.async_resolve(
		ip::tcp::resolver::query(host_, port_),
		[this, self](boost::system::error_code ec, ip::tcp::resolver::iterator it) {
		if (!ec) {
			do_connect(it);
		}
		else {
			session_->html_addcontent(id_, ec.message());
		}
	}
	);
}

void Client::do_connect(ip::tcp::resolver::iterator it)
{
	auto self(shared_from_this());

	async_connect(
		socket_,
		it,
		[this, self](boost::system::error_code ec, ip::tcp::resolver::iterator) {
			if (!ec) {
				do_read();
			}
			else {
				session_->html_addcontent(id_, ec.message());
			}
		}
	);
}

void Client::do_read()
{
	auto self(shared_from_this());

	socket_.async_read_some(
		buffer(recvmsg_, MAX_LENGTH - 1),
		[this, self](boost::system::error_code ec, size_t length) {
			if (!ec) {
				string received(recvmsg_.data(), length);
				session_->html_addcontent(id_, session_->html_escape(received));

				for (auto &ch : received) {
					if (flag_ && ch == ' ') {
						flag_ = false;
						do_write();
					}
					flag_ = (ch == '%');
				}
				do_read();
			}
		}
	);
}

void Client::do_write()
{
	auto self(shared_from_this());
	getline(fin_, sendmsg_);
	sendmsg_ += "\n";

	socket_.async_send(
		buffer(sendmsg_.c_str(), sendmsg_.length()),
		[this, self](boost::system::error_code ec, size_t) {
			if (!ec) {
				session_->html_addcontent(id_, "<b>" + session_->html_escape(sendmsg_) + "</b>");
			}
		}
	);
}

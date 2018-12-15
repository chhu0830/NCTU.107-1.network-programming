#include "console_session.hpp"
#include <iostream>
#include <sstream>
#include "console_client.hpp"

static const std::string CONSOLE(
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
io_service global_io_service;

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

Session::Session(posix::stream_descriptor out, string query) : out_(move(out)), flag_(true)
{
    string s;
    stringstream ss(query);

    while (getline(ss, s, '&')) {
        int pos = s.find("=");
        string var = s.substr(0, pos);
        string val = s.substr(pos+1);

        if (var[0] == 'h') {
            target_["s" + var.substr(1)].host(val);
        } else if (var[0] == 'p') {
            target_["s" + var.substr(1)].port(val);
        } else if (var[0] == 'f') {
            target_["s" + var.substr(1)].file(val);
        }
    }
}

void Session::start()
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

void Session::html_console()
{
    auto self(shared_from_this());
    out_.async_write_some(
        buffer(CONSOLE),
        [self](boost::system::error_code ec, size_t) {
            if (ec) {
                cerr << ec.message() << endl;
            }
        }
    );
} void Session::html_addcontent(string id, string content)
{
    buf_ += "<script>document.getElementById('" + id + "').innerHTML += '" + content + "'</script>";
    if (flag_) {
        flag_ = false;
        html_response();
    }
}

void Session::html_response()
{
    auto self(shared_from_this());
    out_.async_write_some(
        buffer(buf_),
        [this, self](boost::system::error_code ec, size_t length) {
            if (!ec) {
                buf_.erase(0, length);
                if (buf_.length()) {
                    html_response();
                } else {
                    flag_ = true;
                }
            } else {
                cerr << ec.message() << endl;
            }
        }
    );
}

string Session::html_escape(string text)
{
    string out;
    for (auto &ch : text) {
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

Target& Session::target(string id)
{
    return target_[id];
}

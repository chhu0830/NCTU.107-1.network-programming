#include <iostream>
#include <sstream>
#include "console_session.hpp"
#include "console_client.hpp"

Session::Session(string query)
{
    html_template();

    string s;
    stringstream ss(query);
    while (getline(ss, s, '&')) {
        int pos = s.find("=");
        string var = s.substr(0, pos);
        string val = s.substr(pos+1);

        if (var[0] == 'h') {
            _target["s" + var.substr(1)].host(val);
        } else if (var[0] == 'p') {
            _target["s" + var.substr(1)].port(val);
        } else if (var[0] == 'f') {
            _target["s" + var.substr(1)].file(val);
        }
    }

    for (auto &i : _target) {
        if (i.second.valid()) {
            html_addcontent("host", "<th scope=\"col\">" + i.second.host() + ":" + i.second.port() + "</th>");
            html_addcontent("session", "<td><pre id=\"" + i.first + "\" class=\"mb-0\"></pre></td>");

            make_shared<Client>(*this, i.first)->start();
        }
    }
}

void Session::html_template()
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
                    "<tr id=\"host\">\n"
                    "</tr>\n"
                "</thead>\n"
                "<tbody>\n"
                    "<tr id=\"session\">\n"
                    "</tr>\n"
                "</tbody>\n"
            "</table>\n"
        "</body>\n"
        "</html>\n";
}

void Session::html_addcontent(string id, string content)
{
    cout << "<script>" << "document.getElementById('" << id << "').innerHTML += '" << content << "'</script>" << endl;
}

string Session::html_plaintext(string text)
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
    return _target[id];
}

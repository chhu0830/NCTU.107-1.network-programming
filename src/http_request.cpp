#include <regex>
#include <boost/filesystem.hpp>
#include "http_request.hpp"

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

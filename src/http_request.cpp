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
        _request_method = m.str(1);
        _request_uri = m.str(2);
        _script_name = m.str(3);
        _script_filename = boost::filesystem::current_path().string() + _script_name;
        _query_string = m.str(4);
        _server_protocol = m.str(5);
    }

    while (getline(ss, str)) {
        if (regex_match(str, m, regex(HEADER_FIELD))) {
            _headers[m.str(1)] = m.str(2);
        }
    }
}

string& Request::request_method()
{
    return _request_method;
}

string& Request::request_uri()
{
    return _request_uri;
}

string& Request::script_name()
{
    return _script_name;
}

string& Request::script_filename()
{
    return _script_filename;
}

string& Request::query_string()
{
    return _query_string;
}

string& Request::server_protocol()
{
    return _server_protocol;
}

map<string, string>& Request::headers()
{
    return _headers;
}

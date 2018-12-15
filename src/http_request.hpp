#define TOKEN "(?:[-!#$%&'*+.^_`|~a-zA-Z0-9])+"
#define PCHAR "(?:[a-zA-Z0-9-._~!$&'/*+,;=:@]|%[a-fA-F0-9]{2})"
#define VCHAR "(?:[\x21-\x7e])"
#define OWS "(?:[ \t]*)"

#define QUERY "(?:(?:" PCHAR "|[/?])*)"
#define SEGMENT "(?:" PCHAR "*)"
#define ABSOLUTE_PATH "(?:(?:/" SEGMENT ")+)"
#define REQUEST_TARGET "(?:(" ABSOLUTE_PATH ")(?:\\?(" QUERY "))?)"
#define HTTP_VERSION "(?:HTTP/[0-9]\\.[0-9])"
#define METHOD TOKEN
#define REQUEST_LINE "(" METHOD ") (" REQUEST_TARGET ") (" HTTP_VERSION ")\r"

#define FIELD_VALUE "(?:" VCHAR "(?:[ \t]+" VCHAR ")?)*"
#define FIELD_NAME TOKEN
#define HEADER_FIELD "(" FIELD_NAME "):" OWS "(" FIELD_VALUE ")" OWS "\r"

using namespace std;

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


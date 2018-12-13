#include <iostream>
#include "console_session.hpp"
#include "console_client.hpp"

using namespace std;

extern io_service global_io_service;

int main(int, const char *[])
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

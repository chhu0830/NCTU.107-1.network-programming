#include <iostream>
#include "console_session.hpp"
#include "console_client.hpp"

using namespace std;

extern io_service global_io_service;

int main(int, const char *[])
{
    posix::stream_descriptor out(global_io_service, ::dup(STDOUT_FILENO));

    out.async_write_some(
        buffer("Content-Type: text/html\r\n\r\n"),
        [&out](boost::system::error_code ec, size_t) {
            if (!ec) {
                try {
                    make_shared<Session>(move(out), getenv("QUERY_STRING"))->start();
                } catch (exception &e) {
                    cerr << "Exception: " << e.what() << endl;
                }
            } else {
                cerr << ec.message() << endl;
            }
        }
    );

    global_io_service.run();

    return 0;
}

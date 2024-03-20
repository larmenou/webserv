#include "Request.hpp"
#include "DirLister.hpp"

int main()
{
    std::string str("POST /?ha=hello&test=test&ok&&&hello=aa HTTP/1.1\r\nTest-header : header\r\nCookie: oui\r\n\r\nBODY");
    Config conf("./test_configs/cgi.conf");
    DirLister ls;
    Request req(str);

    std::cout << ls.generate_body("./", req) << std::endl;
}
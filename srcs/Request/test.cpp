#include "Request.hpp"

int main()
{
    std::string str("POST /salut?ha=hello&test=test&ok&&&hello=aa HTTP/1.1\r\nTest-header : header\r\nCookie: oui\r\n\r\nBODY");
    Config conf;
    Request req(str);
    std::cout << req;
}
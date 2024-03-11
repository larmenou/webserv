#include "Request.hpp"

int main()
{
    std::string str("POST /salut HTTP/1.1\r\nTest-header : header\r\nCookie: oui\r\nBODY");
    Config conf;
    Request req(str);
    std::cout << req;
}
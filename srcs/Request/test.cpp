#include "Request.hpp"

int main()
{
    std::string str("GET /salut HTTP/1.1\r\nTest-header : header\r\n\r\nBODY");
    Config conf;
    Request req(str);
    std::cout << req;
}
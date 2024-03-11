#include "Request.hpp"

int main()
{
    std::string str("POST /salut HTTP/1.1\n\nBODY");
    Config conf;
    Request req(str);
    std::cout << req;
}
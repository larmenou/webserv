#include "Request.hpp"
#include "CGI.hpp"

int main()
{
    std::string str("POST /cgi/test.php?ha=hello&test=test&ok&&&hello=aa HTTP/1.1\r\nTest-header : header\r\nContent-type: application/x-www-form-urlencoded\r\n\r\nhi=hello");
    std::string conf_path("./test_configs/cgi.conf");
    Config conf;
    conf.initConfig(conf_path);
    Request req(str);
    std::cout << req;

    CGI cgi;
    std::string urn(req.getURN());
    Route r(conf.getServers()[0].findRouteFromURN(urn));
    cgi.prepare(req, r, conf.getServers()[0], "127.0.0.1");
    std::cout << "Result :\n"<< cgi.forwardReq();
}
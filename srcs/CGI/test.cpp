#include "Request.hpp"
#include "CGI.hpp"

static void debug_header(const std::map<std::string, std::string> &headers)
{
    std::map<std::string, std::string>::const_iterator ite;

    std::cout << "Resp headers :" << std::endl;
    for (ite = headers.begin(); ite != headers.end(); ite++)
        std::cout << ite->first << ":" << ite->second << std::endl;
}

int main()
{
    std::string str("POST /cgi/test.php?ha=hello&test=test&ok&&&hello=aa HTTP/1.1\r\nTest-header : header\r\nContent-type: application/x-www-form-urlencoded\r\n\r\nhi=hello&pouet=pouic");
    Config conf("./test_configs/cgi.conf");
    Request req(str);

    CGI cgi;
    std::string urn(req.getURN());
    Route r(conf.getServers()[0].findRouteFromURN(urn));

    cgi.setCGI("./cgi-bin/ubuntu_cgi_tester");
    cgi.prepare(req, r, conf.getServers()[0], "127.0.0.1");
    cgi.forwardReq();
    debug_header(cgi.getHeaders());
    std::cout << "Resp body :\n"<< cgi.getBody() << std::endl << std::endl;

    cgi.setCGI("/usr/bin/php-cgi");
    cgi.prepare(req, r, conf.getServers()[0], "127.0.0.1");
    cgi.forwardReq();
    debug_header(cgi.getHeaders());
    std::cout << "Resp body :\n"<< cgi.getBody() << std::endl << std::endl;
}
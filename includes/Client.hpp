#ifndef _CLIENT_
#define _CLIENT_

#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include "Request.hpp"
#include "CGI.hpp"
#include "DirLister.hpp"
#include "HTTPError.hpp"


#define REQ_TIMEOUT  10

typedef enum e_clientstate
{
    Header,
    Body,
    RespondingHeader,
    RespondingBody,
    Waiting,
    Closed
} t_clientstate;

typedef enum e_reqtype
{
    Cgi,
    Put,
    Post,
    Rewrite,
    Get,
    Delete,
    Error
} t_reqtype;


class Client
{
    private :
        int                 _client_fd;
        bool                _keep_alive;
        t_clientstate       _state;
        t_reqtype           _type;
        int                 _status;
        time_t              _start;
        Request             _req;
        CGI                 _cgi;
        ServerConf          _server;
        Route               _route;
        const Config        *_conf;
        std::string         _ip;
        std::map<t_reqtype,void (Client::*)(char const *, size_t)>   _body_functions;
        std::map<t_reqtype,void (Client::*)()>   _reponse_functions;

        std::string         _body_response;
        std::string         _headers;

        int                 _fd;
        ssize_t             _bodyc;
        ssize_t             _body_len;
        char                _buff[BUFFER_SIZE];

        Client();

        void    sendResponse();
        void    processBody(char const *chunk,
                            size_t start);
        void    bodyPostGet(char const *chunk, size_t start);
        void    bodyPut(char const *chunk, size_t start);
        void    bodyCgi(char const *chunk, size_t start);
        void    bodyRewrite(char const *chunk, size_t start);
        void    bodyDelete(char const *chunk, size_t start);
        void    bodyError(char const *chunk, size_t start);

        void    responsePostGet();
        void    responsePut();
        void    responseCgi();
        void    responseRewrite();
        void    responseDelete();
        void    responseError();

        void    sendFile();
        void    sendHeader();

        void    reset();

        std::string buildFilename();

        void    determineRequestType();
        void    initServerRoute();
        void    buildHeaderConnection(std::stringstream &http);

    public :
        Client(int client_fd,
                const Config *conf,
                std::string server_ip);
        Client(Client const &client);
        ~Client();
        Client  &operator=(Client const &client);

        void    receive();
        void    respond();
        bool    isExpired();
        int     getFD();
        t_clientstate getState();
};

#endif
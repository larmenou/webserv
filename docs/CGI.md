# CGI class

CGI is a class that forwards a request to a set CGI and allows us to retrieve it's response.

## Public methods

### Constructors
- `CGI()` : Default constructor

## Utils

- `void    CGI::setCGI(std::string cgiPath)` : sets the path to the binary of the CGI that'll handle the request.
- `void    CGI::prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr)` : prepares a request to the CGI by initalising the environment with the appropriate values given a Route, a ServerConf and the remote address of the client.
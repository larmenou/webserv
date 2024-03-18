# ServerConf class

ServerConf is a class that allows the server to store server directives of conf files. 

## Public methods

### Constructors
- `ServerConf()` : Default constructor which initalizes the ip to "0.0.0.0", root to "./" and directory listing to `true`.
- `ServerConf(ServerConf const &a)` : Copy constructor

### Utils 
- `const Route findRouteFromURN(std::string urn) const` : Finds and returns the route that is associated to URN, throws a std::runtime_error if no route is found.

Example :
```cpp
const Route &route = server.findRouteFromURN("/redirect?test=aaaa");
std::cout << route << std::endl;
```
### Getters


- `void        setIP(std::string ip)`
- `void        setRoutes(std::vector<Route> &routes)`
- `void        setPort(int port)`
- `void        setBodySizeLimit(size_t size)`
- `void        addServerName(std::string name)`
- `void        addErrorPage(unsigned int code, std::string path)`
- `void        setRoot(std::string root)`
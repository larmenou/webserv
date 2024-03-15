# Config class

Config is a class that allows the server to parse .conf files following an nginx-like syntax and stores the parsed information into a vector of ServerConf objects.

## Public methods

### Constructors
- `Config(std::string &config_path)` : Constructor that directly initializes the object by parsing a file located at `config_path`.

### Methods
- `void    initConfig(std::string &config_path)` : Method that parses 
- `const ServerConf    &getServerFromHostAndIP(std::string host, std::string ip)` : Finds and returns the corresponding constant reference to the host from an IP and the Host header.

Example :
```cpp
Config  conf("./test_configs/cgi.conf");
const ServerConf  &s_conf = getServerFromHostAndIP("foo.com", "127.0.0.1");
std::cout << s_conf << std::endl
```

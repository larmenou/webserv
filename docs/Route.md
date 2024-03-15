# Route class

Route is a class that stores a route's informations.

## Public methods

### Constructors
- `Route()` : Default constructor which initalizes the method permissions to `POST or GET` and directory listing to false.
- `Route(Route const &a)` : Copy constructor


### Getters
- `const std::string   &getRoute() const`
- `const std::string   &getRoot() const`
- `const std::string   &getDirFile() const`
- `const std::string   &getCgiExtension() const`
- `const std::string   &getSavePath() const`
- `const std::pair<std::string, std::string> &getRewrite() const`
- `long    getRedirCode() const`
- `long    getMethodPerms() const`
- `bool    isAcceptingUploads() const`
- `bool    isListingDirs() const`

### Setters
- `void    setRoute(std::string route)`
- `void    setRoot(std::string root)`
- `void    setDirFile(std::string dirFile)`
- `void    setCgiExtension(std::string getCgiExtension)`
- `void    setSavePath(std::string savePath)`
- `void    setRedirection(std::string from, std::string to, long code)`
- `void    setMethodPerms(long perms)`
- `void    setUpload(bool acceptsUploads)`
- `void    setListDirectory(bool listDir)`

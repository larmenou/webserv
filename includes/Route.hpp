#ifndef _ROUTE_
#define _ROUTE_

# define GET        0b1
# define POST       0b10
# define PUT        0b100
# define OPTIONS    0b1000
# define HEAD       0b10000
# define CONNECT    0b100000
# define DELETE     0b1000000
# define PATCH      0b10000000
# define TRACE      0b100000000

#include <iostream>
#include <utility>

class Route
{
    private :
        std::string     _route;
        long            _methodPerms;
        std::string     _root;
        std::pair<std::string, std::string>     _rewrite;
        long                                    _redir_code;
        bool            _dirListing;
        std::string     _isDirFile;
        std::string     _cgiFileExtension;

        bool            _acceptUploads;
        std::string     _fileSavePath;
    
    public :
        Route();
        Route(Route const &a);
        ~Route();
        Route &operator=(Route const &a);

        const std::string   &getRoute() const;
        const std::string   &getRoot() const;
        const std::string   &getDirFile() const;
        const std::string   &getCgiExtension() const;
        const std::string   &getSavePath() const;
        const std::pair<std::string, std::string> &getRewrite() const;
        long    getRedirCode() const;
        long    getMethodPerms() const;
        bool    isAcceptingUploads() const;
        bool    isListingDirs() const;

        void    setRoute(std::string route);
        void    setRoot(std::string root);
        void    setDirFile(std::string dirFile);
        void    setCgiExtension(std::string getCgiExtension);
        void    setSavePath(std::string savePath);
        void    setRedirection(std::string from, std::string to, long code);
        void    setMethodPerms(long perms);
        void    setUpload(bool acceptsUploads);
        void    setListDirectory(bool listDir);
};

std::ostream    &operator<<(std::ostream &os, const Route &route);

#endif
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
        bool            _dirListing;
        std::string     _isDirFile;
        std::string     _cgiFileExtension;

        bool            _acceptUploads;
        std::string     _fileSavePath;
        Route();

    public :
        Route(long _methodPerms,
                std::string &root,
                std::pair<std::string, std::string> &rewrite,
                bool dirListing,
                std::string &dirFile,
                std::string &cgiFileExtension,
                bool acceptUploads,
                std::string &fileSavePath);
        Route(Route const &a);
        ~Route();
        Route &operator=(Route const &a);
};

#endif
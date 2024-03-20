#ifndef _DIRLS_
#define _DIRLS_

#include <set>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include "Request.hpp"

class DirLister
{
    private :
        std::set<std::string> _files;

        DirLister(DirLister const &a);
        DirLister &operator=(DirLister const &a);

        void    list(std::string path);
    public :
        DirLister();
        ~DirLister();

        std::string generate_body(std::string path, const Request &req);
};

#endif
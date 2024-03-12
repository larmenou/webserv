#ifndef _PhpCGI_
#define _PhpCGI_

#include <iostream>
#include <sstream>
#include "Request.hpp"

class PhpCGI
{
    public :
        PhpCGI();
        ~PhpCGI();

        static std::string  forwardReq(std::string &scriptPath, Request &request);
};

#endif
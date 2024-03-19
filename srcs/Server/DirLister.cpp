#include "DirLister.hpp"

DirLister::DirLister()
{}

DirLister::~DirLister()
{}

void    DirLister::list(std::string path)
{
    DIR     *ds;
    dirent  *de;

    _files.clear();
    ds = opendir(path.c_str());
    if (ds == NULL)
        throw std::exception();
    while(true)
    {
        errno = 0;
        de = readdir(ds);
        if (de == NULL)
            break;
        _files.insert(de->d_name);
    }
    if (errno != 0)
    {
        closedir(ds);
        throw std::exception();
    }
    closedir(ds);
}

std::string DirLister::generate_body(std::string path, const Request &req)
{
    std::stringstream ss;

    list(path);
    ss << "<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    ss << "<title>Directory listing for " << req.getURN() <<" </title>\n</head>\n<body>\n";
    ss << "<h1> Directory listing for " << req.getURN() << " </h1>\n";
    ss << "<hr><ul>\n";
    for (std::set<std::string>::iterator ite = _files.begin(); ite != _files.end(); ite++)
    {
        if (*ite != "." && *ite != "..")
            ss << "<li><a href=\"" << *ite << "\" > " << *ite << " </a></li>\n";
    }
    ss << "</ul><hr>";
    ss <<"</body>\n</html>";
    return ss.str();
}



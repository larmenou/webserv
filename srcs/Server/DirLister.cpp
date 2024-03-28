#include "DirLister.hpp"

DirLister::DirLister()
{}

DirLister::~DirLister()
{}

void    DirLister::list(std::string path)
{
    DIR     *ds;
    dirent  *de;
    std::string name;

    _files.clear();
    ds = opendir(path.c_str());
    if (ds == NULL)
        throw std::runtime_error("403");
    while(true)
    {
        errno = 0;
        de = readdir(ds);
        if (de == NULL)
            break;
        name = de->d_name;
        if (de->d_type == DT_DIR)
            name += '/';
        _files.insert(name);
    }
    if (errno != 0)
    {
        closedir(ds);
        throw std::runtime_error("500");
    }
    closedir(ds);
}

std::string DirLister::generate_body(std::string path, const Request &req)
{
    std::stringstream ss;

    list(path);
    ss << "<html>\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n";
    ss << "<title>Directory listing for " << req.getPath() <<" </title>\n</head>\n<body>\n";
    ss << "<h1> Directory listing for " << req.getPath() << " </h1>\n";
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



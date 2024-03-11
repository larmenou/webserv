#include "Route.hpp"

Route::Route() :
                    _route(),
                    _methodPerms(POST | GET),
                    _root(),
                    _rewrite(),
                    _redir_code(-1),
                    _dirListing(false),
                    _isDirFile(),
                    _cgiFileExtension(),
                    _acceptUploads(),
                    _fileSavePath()
{

}

Route::Route(Route const &a)
{
    *this = a;
}

Route   &Route::operator=(Route const &a)
{
    _methodPerms = a._methodPerms;
    _root = a._root;
    _rewrite = a._rewrite;
    _redir_code = a._redir_code;
    _dirListing = a._dirListing;
    _isDirFile = a._isDirFile;
    _cgiFileExtension = a._acceptUploads;
    _fileSavePath = a._fileSavePath;
    _route = a._route;
    _acceptUploads = a._acceptUploads;

    return *this;
}

Route::~Route()
{
}

const std::string   &Route::getRoute() const { return _route; }
const std::string   &Route::getRoot() const { return _root;}
const std::string   &Route::getDirFile() const { return _isDirFile; }
const std::string   &Route::getCgiExtension() const { return _cgiFileExtension; }
const std::string   &Route::getSavePath() const { return _fileSavePath; }
const std::pair<std::string, std::string> &Route::getRewrite() const { return _rewrite; }
long    Route::getRedirCode() const { return _redir_code; }
long    Route::getMethodPerms() const { return _methodPerms; }
bool    Route::isAcceptingUploads() const { return _acceptUploads; }
bool    Route::isListingDirs() const { return _dirListing; }

void    Route::setRoute(std::string &route) { _route = route; }
void    Route::setRoot(std::string &root) { _root = root; }
void    Route::setDirFile(std::string &dirFile) {_isDirFile = dirFile; }
void    Route::setCgiExtension(std::string &getCgiExtension) { _cgiFileExtension = getCgiExtension; }
void    Route::setSavePath(std::string &savePath) { _fileSavePath = savePath; }
void    Route::setRedirection(std::string &from, std::string &to, long code)
{
    _rewrite.first = from;
    _rewrite.second = to;
    _redir_code = code;
}
void    Route::setMethodPerms(long perms) { _methodPerms = perms; }
void    Route::setUpload(bool acceptsUploads) { _acceptUploads = acceptsUploads; }
void    Route::setListDirectory(bool listDir) { _dirListing = listDir; }

std::ostream    &operator<<(std::ostream &os, const Route &route)
{
    os << "Route '" << route.getRoute() << "' with";
    if (route.getRoot().length() != 0)
        os << "\n\t- root at '" << route.getRoot() << "'";
    if (route.getRedirCode() != -1)
        os << "\n\t- "<< (route.getRedirCode() == 301 ? "permanent " : "") 
        << "redirection from '" << route.getRewrite().first << 
        "' to '" << route.getRewrite().second << "'";
    os << "\n\t- autoindex " << (route.isListingDirs() ? "on" : "off");
    if (route.isAcceptingUploads())
        os << "\n\t- accepting upload at " << route.getSavePath();
    return os;
}
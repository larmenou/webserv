#include "Route.hpp"

Route::Route() :
                    _route(),
                    _methodPerms(POST | GET),
                    _root(),
                    _rewrite(),
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
    _dirListing = a._dirListing;
    _isDirFile = a._isDirFile;
    _cgiFileExtension = a._acceptUploads;
    _fileSavePath = a._fileSavePath;

    return *this;
}

Route::~Route()
{
}

const std::string   &Route::getRoute() { return _route; }
const std::string   &Route::getRoot() { return _root;}
const std::string   &Route::getDirFile() { return _isDirFile; }
const std::string   &Route::getCgiExtension() { return _cgiFileExtension; }
const std::string   &Route::getSavePath() { return _fileSavePath; }
const std::pair<std::string, std::string> &Route::getRewrite() { return _rewrite; }
long    Route::getMethodPerms() { return _methodPerms; }
bool    Route::isAcceptingUploads() { return _acceptUploads; }
bool    Route::isListingDirs() { return _dirListing; }

void    Route::setRoute(std::string &route) { _route = route; }
void    Route::setRoot(std::string &root) { _root = root; }
void    Route::setDirFile(std::string &dirFile) {_isDirFile = dirFile; }
void    Route::setCgiExtension(std::string &getCgiExtension) { getCgiExtension = getCgiExtension; }
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
#include "Route.hpp"

Route::Route(long methodPerms,
                std::string &root,
                std::pair<std::string, std::string> &rewrite,
                bool dirListing,
                std::string &dirFile,
                std::string &cgiFileExtension,
                bool acceptUploads,
                std::string &fileSavePath) :
                    _methodPerms(methodPerms),
                    _root(root),
                    _rewrite(rewrite),
                    _dirListing(dirListing),
                    _isDirFile(dirFile),
                    _cgiFileExtension(cgiFileExtension),
                    _acceptUploads(acceptUploads),
                    _fileSavePath(fileSavePath)
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
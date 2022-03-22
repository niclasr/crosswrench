#if !defined(_SRC_SPREAD_HPP_)
#define _SRC_SPREAD_HPP_

#include "record.hpp"

#include <libzippp.h>

#include <filesystem>
#include <string>

namespace crosswrench {

class spread
{
  public:
    spread(libzippp::ZipArchive &, bool isrootpurelib);
    void install();

  private:
    void installdata(libzippp::ZipEntry &);
    void installdotdatadir(libzippp::ZipEntry &);
    void installentry(libzippp::ZipEntry &);
    void installinclude(libzippp::ZipEntry &);
    void installplatinclude(libzippp::ZipEntry &);
    void installplatlib(libzippp::ZipEntry &);
    void installpurelib(libzippp::ZipEntry &);
    void installroot(libzippp::ZipEntry &);
    void installscripts(libzippp::ZipEntry &);

    libzippp::ZipArchive &wheelfile;
    record record2write;
    bool rootispurelib;
    // std::map<std::string, void(*)(libzippp::ZipEntry &)>
    //   datadirinstallers;
    std::filesystem::path destdir;
    std::ios_base::openmode outmode;
};

} // namespace crosswrench

#endif

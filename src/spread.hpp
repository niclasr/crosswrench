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
    void installdotdatadir(libzippp::ZipEntry &);
    void installentry(libzippp::ZipEntry &);
    void installfile(libzippp::ZipEntry &, std::filesystem::path);

    libzippp::ZipArchive &wheelfile;
    record record2write;
    bool rootispurelib;
    std::filesystem::path destdir;
    std::ios_base::openmode outmode;
};

} // namespace crosswrench

#endif

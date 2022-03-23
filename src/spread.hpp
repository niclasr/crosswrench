#if !defined(_SRC_SPREAD_HPP_)
#define _SRC_SPREAD_HPP_

#include "record.hpp"

#include <libzippp.h>

#include <filesystem>
#include <set>
#include <string>

namespace crosswrench {

class spread
{
  public:
    spread(libzippp::ZipArchive &, bool isrootpurelib);
    void install();

  private:
    void compile();
    void installdotdatadir(libzippp::ZipEntry &);
    void installentry(libzippp::ZipEntry &);
    void installfile(libzippp::ZipEntry &, std::filesystem::path, bool = false);

    libzippp::ZipArchive &wheelfile;
    record record2write;
    bool rootispurelib;
    std::filesystem::path destdir;
    std::ios_base::openmode outmode;
    std::set<std::filesystem::path> py_files;
};

} // namespace crosswrench

#endif

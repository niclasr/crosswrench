#if !defined(_SRC_SPREAD_HPP_)
#define _SRC_SPREAD_HPP_

#include "hashlib2botan.hpp"
#include "record.hpp"

#include <botan/hash.h>
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
    void add2record(std::filesystem::path,
                    std::unique_ptr<Botan::HashFunction> &);
    void compile();
    void createdirs(std::filesystem::path);
    std::filesystem::path createinstallpath(std::filesystem::path,
                                            std::filesystem::path);
    std::filesystem::path dotdatadirinstallpath(libzippp::ZipEntry &);
    std::filesystem::path installpath(libzippp::ZipEntry &);
    void installfile(libzippp::ZipEntry &, std::filesystem::path);
    void installfile(const char *, size_t, std::filesystem::path);
    void installinstallerfile();
    uintptr_t writereplacedpython(const void *,
                                  libzippp_uint64,
                                  std::unique_ptr<Botan::HashFunction> &,
                                  std::ofstream &);

    libzippp::ZipArchive &wheelfile;
    record record2write;
    bool rootispurelib;
    std::filesystem::path destdir;
    std::ios_base::openmode outmode;
    std::set<std::filesystem::path> py_files;
    hashlib2botan h2b;
};

} // namespace crosswrench

#endif

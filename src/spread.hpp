#if !defined(_SRC_SPREAD_HPP_)
#define _SRC_SPREAD_HPP_

#include "hashlib2botan.hpp"
#include "record.hpp"

#include <boost/filesystem.hpp>
#include <botan/hash.h>
#include <libzippp.h>

#include <set>
#include <string>

namespace crosswrench {

class spread
{
  public:
    spread(libzippp::ZipArchive &, bool isrootpurelib);
    void install();

  private:
    void add2record(boost::filesystem::path,
                    std::unique_ptr<Botan::HashFunction> &);
    void compile();
    void createdirs(boost::filesystem::path);
    boost::filesystem::path createinstallpath(boost::filesystem::path,
                                            boost::filesystem::path);
    boost::filesystem::path dotdatadirinstallpath(libzippp::ZipEntry &);
    boost::filesystem::path installpath(libzippp::ZipEntry &);
    void installfile(libzippp::ZipEntry &, boost::filesystem::path);
    void installfile(const char *, size_t, boost::filesystem::path);
    void installinstallerfile();
    uintptr_t writereplacedpython(const void *,
                                  libzippp_uint64,
                                  std::unique_ptr<Botan::HashFunction> &,
                                  std::ofstream &);
    void installentrypointconsolescripts();
    void installdirecturl();
    void printverboseinstallloc(std::string, std::string);
    void checkinstallaccess(boost::filesystem::path);
    boost::filesystem::path installpath(std::string);

    libzippp::ZipArchive &wheelfile;
    record record2write;
    bool rootispurelib;
    boost::filesystem::path destdir;
    std::ios_base::openmode outmode;
    std::set<boost::filesystem::path> py_files;
    hashlib2botan h2b;
    bool verbose;
};

} // namespace crosswrench

#endif

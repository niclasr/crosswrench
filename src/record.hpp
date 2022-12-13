#if !defined(_SRC_RECORD_HPP_)
#define _SRC_RECORD_HPP_

#include <boost/filesystem.hpp>
#include <libzippp.h>

#include <array>
#include <map>
#include <string>

namespace crosswrench {

class record
{
  public:
    record() = delete;
    record(std::string);
    bool verify(libzippp::ZipArchive &);
    bool add(std::string, std::string, std::string, std::string);
    void write(bool, boost::filesystem::path);

  private:
    std::map<std::string, std::array<std::string, 3>> records;
};

} // namespace crosswrench
#endif

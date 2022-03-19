#if !defined(_SRC_RECORD_HPP_)
#define _SRC_RECORD_HPP_

#include <libzippp.h>

#include <map>
#include <string>

namespace crosswrench {

class record
{
  public:
    record() = delete;
    record(std::string);
    bool verify(libzippp::ZipArchive&);

  private:
    std::map<std::string, std::array<std::string, 3>> records;
};

} // namespace crosswrench
#endif

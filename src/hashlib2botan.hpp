
#if !defined(_SRC_HASHLIB2BOTAN_HPP_)
#define _SRC_HASHLIB2BOTAN_HPP_

#include <map>
#include <string>

namespace crosswrench {
class hashlib2botan
{
  public:
    hashlib2botan();
    bool available(std::string);
    std::string hashname(std::string);

  private:
    std::map<std::string, std::string> conv_table;
};
} // namespace crosswrench

#endif


#if !defined(_SRC_HASHLIB2BOTAN_HPP_)
#define _SRC_HASHLIB2BOTAN_HPP_

#include <array>
#include <map>
#include <string>
#include <vector>

namespace crosswrench {
class hashlib2botan
{
  public:
    hashlib2botan();
    bool available(std::string);
    std::string hashname(std::string);
    std::string strongest_algorithm_hashlib();
    std::string strongest_algorithm_botan();
    void print_guaranteed();

  private:
    std::map<std::string, std::string> conv_table;
    std::vector<std::string> algorithms_guaranteed;
    std::array<std::string, 4> algorithms_by_strength;
    std::string best_algo;
};
} // namespace crosswrench

#endif

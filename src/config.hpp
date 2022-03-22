#if !defined(_SRC_CONFIG_HPP_)
#define _SRC_CONFIG_HPP_

#include <cxxopts.hpp>

#include <map>
#include <string>

namespace crosswrench {

class config
{
  public:
    static config *instance();
    bool setup(cxxopts::ParseResult &);
    bool setup(std::map<std::string, std::string> &);
    std::string get_value(std::string);
    void print_all();
    config(const config &) = delete;
    config &operator=(const config &) = delete;

  private:
    bool set_python_value(std::string, cxxopts::ParseResult &);
    bool verify_python_interpreter(cxxopts::ParseResult &pr);
    config() = default;
    std::map<std::string, std::string> db;
    std::map<std::string, std::string> new_db;
};

} // namespace crosswrench

#endif

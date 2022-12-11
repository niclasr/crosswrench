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
    std::string dotdatakeydir2config(std::string &);

  private:
    bool get_algos(cxxopts::ParseResult &);
    bool get_framework(cxxopts::ParseResult &);
    std::string get_scheme(std::string &);
    bool verify_python_interpreter(cxxopts::ParseResult &pr);
    bool get_python_paths(cxxopts::ParseResult &);
    config();
    std::map<std::string, std::string> db;
    std::map<std::string, std::string> new_db;
    std::map<std::string, std::string> dotdatakeydir2config_map;
    bool _framework;
};

} // namespace crosswrench

#endif

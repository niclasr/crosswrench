#if !defined(_SRC_FUNCTIONS_HPP_)
#define _SRC_FUNCTIONS_HPP_

#include <string>

namespace crosswrench {
std::string dotdistinfodir();
std::string dotdatadir();
bool isbase64nopad(const std::string&);
} // namespace crosswrench

#endif

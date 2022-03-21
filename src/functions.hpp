#if !defined(_SRC_FUNCTIONS_HPP_)
#define _SRC_FUNCTIONS_HPP_

#include <libzippp.h>

#include <string>

namespace crosswrench {
std::string dotdistinfodir();
std::string dotdatadir();
bool isbase64urlsafenopad(const std::string&);
bool isversionnumber(const std::string&);
bool iswheelfilenamevalid(const std::string&);
bool minimumdistinfofiles(libzippp::ZipArchive&);
std::string base64urlsafenopad(std::string);
} // namespace crosswrench

#endif

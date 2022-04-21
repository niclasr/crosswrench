#if !defined(_SRC_FUNCTIONS_HPP_)
#define _SRC_FUNCTIONS_HPP_

#include <libzippp.h>

#include <filesystem>
#include <string>

namespace crosswrench {
std::string dotdistinfodir();
std::string dotdatadir();
bool isbase64urlsafenopad(const std::string &);
bool isversionnumber(const std::string &);
bool iswheelfilenamevalid(const std::string &);
bool minimumdistinfofiles(libzippp::ZipArchive &);
std::string base64urlsafenopad(std::string);
bool isrecordfilenames(std::string);
std::filesystem::path rootinstalldir(bool);
std::filesystem::path installdir(std::string);
bool get_cmd_output(std::string &, std::string &, std::string);
bool wheelhasabsolutepaths(libzippp::ZipArchive &);
bool onlyalloweddotdatapaths(libzippp::ZipArchive &ar);
std::filesystem::path dotdatainstalldir(std::string);
bool isscript(libzippp::ZipEntry &);
bool strvec_contains(std::vector<std::string> &, std::string &);
uint16_t getelf16(uint8_t, const uint8_t *);
uint32_t getelf32(uint8_t, const uint8_t *);
bool iselfexec(libzippp::ZipEntry &);
} // namespace crosswrench

#endif

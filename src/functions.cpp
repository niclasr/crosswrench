/*
Copyright (c) 2022 Niclas Rosenvik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "functions.hpp"

#include "config.hpp"

#include <boost/filesystem.hpp>
#include <cxxopts.hpp>
#include <libzippp.h>
#include <pstream.h>
#include <pystring.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace crosswrench {

namespace {
std::string
distdashversion()
{
    std::vector<std::string> result;
    std::string wheelarg = config::instance()->get_value("wheel");
    boost::filesystem::path wheelpath{ wheelarg };
    std::string wheelname = wheelpath.filename().string();
    pystring::split(wheelname, result, "-");
    result.erase(result.begin() + 2, result.end());

    return pystring::join("-", result);
}

std::map<std::string, std::string> opt2env{ { "destdir", "DESTDIR" },
                                            { "python", "PYTHONBIN" } };

} // namespace

std::string
dotdistinfodir()
{
    return distdashversion() + ".dist-info";
}

std::string
dotdatadir()
{
    return distdashversion() + ".data";
}

bool
isbase64urlsafenopad(const std::string &str)
{
    auto pred = [](unsigned char c) {
        return (std::isalpha(c) || std::isdigit(c) || c == '-' || c == '_');
    };

    return std::all_of(str.cbegin(), str.cend(), pred);
}

bool
iswheelfilenamevalid(const std::string &filepath)
{
    boost::filesystem::path wheelpath{ filepath };
    std::string wheelname = wheelpath.filename().string();

    if (!pystring::endswith(wheelname, ".whl")) {
        return false;
    }

    std::vector<std::string> splited;
    pystring::split(wheelname, splited, "-");

    if (!(splited.size() == 6 || splited.size() == 5)) {
        return false;
    }

    // the build tag must start with a number
    if (splited.size() == 6) {
        return pystring::isdigit(splited.at(3).substr(0, 1));
    }

    return true;
}

bool
minimumdistinfofiles(libzippp::ZipArchive &ar)
{
    std::array<std::string, 3> reqfiles{ "/METADATA", "/RECORD", "/WHEEL" };
    auto pred = [&](std::string &filename) {
        return ar.hasEntry(dotdistinfodir() + filename);
    };
    return std::all_of(reqfiles.begin(), reqfiles.end(), pred);
}

std::string
base64urlsafenopad(std::string b64str)
{
    auto usnpb64 = pystring::rstrip(b64str, "=");
    std::replace(usnpb64.begin(), usnpb64.end(), '+', '-');
    std::replace(usnpb64.begin(), usnpb64.end(), '/', '_');
    return usnpb64;
}

bool
isrecordfilenames(std::string name)
{
    return (name == dotdistinfodir() + "/RECORD") ||
           (name == dotdistinfodir() + "/RECORD.p7s") ||
           (name == dotdistinfodir() + "/RECORD.jws");
}

boost::filesystem::path
rootinstalldir(bool rootispurelib)
{
    std::string rootinstall = rootispurelib ? "purelib" : "platlib";
    return installdir(rootinstall);
}

boost::filesystem::path
dotdatainstalldir(std::string keydir)
{
    std::string dotdatainstall =
      config::instance()->dotdatakeydir2config(keydir);
    return installdir(dotdatainstall);
}

boost::filesystem::path
installdir(std::string pythondir)
{
    boost::filesystem::path thepath = config::instance()->get_value(pythondir);
    return thepath.relative_path();
}

bool
get_cmd_output(std::string &cmd,
               std::vector<std::string> &output,
               std::string pipein)
{
    unsigned int exit_rounds = 0;
    redi::pstreams::pmode pm =
      pipein.empty() ? redi::pstreams::pstdout
                     : redi::pstreams::pstdin | redi::pstreams::pstdout;
    redi::pstream in(cmd, pm);
    if (!pipein.empty()) {
        in << pipein;
        in << redi::peof;
    }

    std::string outputline;
    while (std::getline(in.out(), outputline)) {
        output.push_back(outputline);
    }
    in.close();

    while (!in.rdbuf()->exited()) {
        if (exit_rounds > 4) {
            if (in.rdbuf()->kill() == nullptr) {
                in.rdbuf()->kill(SIGKILL);
            }
            std::cerr << "the command: " << in.command()
                      << " did not finish in time" << std::endl;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        exit_rounds++;
    }

    if (in.rdbuf()->status() != 0) {
        std::cerr << "the command: " << in.command() << " did not exit with 0"
                  << std::endl;
        return false;
    }

    return true;
}

bool
wheelhasabsolutepaths(libzippp::ZipArchive &ar)
{
    auto pred = [](libzippp::ZipEntry &e) {
        return pystring::startswith(e.getName(), "/");
    };

    auto entries = ar.getEntries();

    return std::any_of(entries.begin(), entries.end(), pred);
}

bool
onlyalloweddotdatapaths(libzippp::ZipArchive &ar)
{
    auto entries = ar.getEntries();

    for (auto entry : entries) {
        if (entry.isDirectory()) {
            continue;
        }

        if (pystring::startswith(entry.getName(), dotdatadir() + "/")) {
            std::vector<std::string> dirnames;
            pystring::split(entry.getName(), dirnames, "/");
            auto cfgname =
              config::instance()->dotdatakeydir2config(dirnames[1]);
            if (cfgname.empty()) {
                return false;
            }

            // check that the fileentry is under a direcory in .data
            if (!pystring::startswith(entry.getName(),
                                      dirnames[0] + "/" + dirnames[1] + "/"))
            {
                return false;
            }
        }
    }

    return true;
}

bool
isscript(libzippp::ZipEntry &entry)
{
    return pystring::startswith(entry.getName(), dotdatadir() + "/scripts/");
}

bool
strvec_contains(std::vector<std::string> &v, std::string &str)
{
    auto result = std::find(v.begin(), v.end(), str);
    return result != v.end();
}

std::uint16_t
getelf16(std::uint8_t endian, const std::uint8_t *data)
{
    if (endian == 1) { // little endian
        return (data[0] << 0) | (data[1] << 8);
    }
    else {
        return (data[1] << 0) | (data[0] << 8);
    }
}

std::uint32_t
getelf32(std::uint8_t endian, const std::uint8_t *data)
{
    if (endian == 1) { // little endian
        return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) |
               (data[3] << 24);
    }
    else {
        return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) |
               (data[0] << 24);
    }
}

bool
iselfexec(libzippp::ZipEntry &entry)
{
    const std::uint8_t elf_magic[] = { 0x7F, 0x45, 0x4c, 0x46 };

    const std::uint8_t elf_32bit = 1;
    const std::uint8_t elf_64bit = 2;
    const std::uint8_t elf_little = 1;
    const std::uint8_t elf_big = 2;
    const std::uint16_t elf_exec = 0x02;
    const std::uint16_t elf_shared = 0x03;
    const size_t elf_hdrsize = 0x40;

    if (entry.getSize() < elf_hdrsize) {
        return false;
    }

    std::uint8_t *data =
      (std::uint8_t *)entry.readAsBinary(libzippp::ZipArchive::Original,
                                    elf_hdrsize);

    int magic = std::memcmp(data, elf_magic, sizeof(elf_magic));

    std::uint8_t ei_class = data[0x04];
    std::uint8_t ei_endian = data[0x05];
    std::uint8_t ei_version = data[0x06];
    std::uint16_t e_type = getelf16(ei_endian, data + 0x10);
    std::uint32_t e_version = getelf32(ei_endian, data + 0x14);

    delete[] data;

    return (magic == 0) && (ei_class == elf_32bit || ei_class == elf_64bit) &&
           (ei_endian == elf_little || ei_endian == elf_big) &&
           (ei_version == 1) && (e_type == elf_exec || e_type == elf_shared) &&
           (e_version == 1);
}

std::map<std::string, std::string>
getentrypointscripts(libzippp::ZipEntry &entry)
{
    std::map<std::string, std::string> scripts;

    auto file = entry.readAsText();
    std::vector<std::string> lines;
    pystring::splitlines(file, lines);

    std::for_each(lines.begin(), lines.end(), [](std::string &l) {
        l = pystring::strip(l);
    });

    std::array<std::string, 2> sections{ "[console_scripts]", "[gui_scripts]" };
    for (auto section : sections) {
        auto sec_start = std::find(lines.begin(), lines.end(), section);
        if (sec_start != lines.end()) {
            sec_start++; // we must start on the line after sec_start since
                         // sec_start starts with [
            auto sec_end =
              std::find_if(sec_start, lines.end(), [](std::string &l) {
                  return pystring::startswith(l, "[");
              });
            for (auto i = sec_start; i != sec_end; i++) {
                if (i->empty() || pystring::startswith(*i, "#") ||
                    (pystring::find(*i, "[") != -1))
                {
                    continue;
                }

                std::vector<std::string> result;
                pystring::partition(*i, "=", result);
                auto prefix = config::instance()->get_value("script-prefix");
                auto suffix = config::instance()->get_value("script-suffix");
                scripts[prefix + pystring::strip(result.at(0)) + suffix] =
                  createscript(result.at(2));
            }
        }
    }

    return scripts;
}

std::string
createscript(std::string &rhs)
{
    std::vector<std::string> result;
    pystring::partition(rhs, ":", result);
    std::string package = pystring::strip(result.at(0));
    std::string function = pystring::strip(result.at(2));

    std::string script;
    script += "#!" + config::instance()->get_value("python") + "\n";
    script += "import sys\n";
    script += "from " + package + " import " + function + "\n";
    script += "sys.exit(" + function + "())\n";

    return script;
}

void
setexecperms(boost::filesystem::path filepath)
{
    boost::filesystem::permissions(filepath,
                                 boost::filesystem::perms::owner_exe |
                                   boost::filesystem::perms::group_exe |
                                   boost::filesystem::perms::others_exe |
                                   boost::filesystem::perms::add_perms);
}

bool
wheelhasdotdotpath(libzippp::ZipArchive &ar)
{
    std::string dotdot{ ".." };
    auto entries = ar.getEntries();
    for (auto entry : entries) {
        auto name = entry.getName();
        std::vector<std::string> namesplited;
        pystring::split(name, namesplited, "/");

        if (strvec_contains(namesplited, dotdot)) {
            return true;
        }
    }

    return false;
}

std::string
expandhome(std::string path)
{
    if (pystring::startswith(path, "~/")) {
        char *envstr = std::getenv("HOME");
        if (envstr != nullptr) {
            std::string home{ envstr };
            path = home + path.substr(1);
        }
    }

    return path;
}

int
countoptorenv(cxxopts::ParseResult &pr, std::string opt)
{
    if (pr.count(opt)) {
        return pr.count(opt);
    }

    auto v = opt2env.find(opt);
    if (v != opt2env.end()) {
        if (std::getenv(v->second.c_str()) != nullptr) {
            return 1;
        }
    }

    return 0;
}

std::string
getoptorenv(cxxopts::ParseResult &pr, std::string opt)
{
    if (pr.count(opt)) {
        return pr[opt].as<std::string>();
    }

    auto v = opt2env.find(opt);
    if (v != opt2env.end()) {
        char *envstr = std::getenv(v->second.c_str());
        if (envstr != nullptr) {
            return std::string{ envstr };
        }
    }

    if (pr[opt].has_default()) {
        return pr[opt].as<std::string>();
    }

    return "";
}

std::string
envormsg(std::string &opt)
{
    std::vector<std::string> vmsg;
    vmsg.push_back(" or environment variable ");

    return envmsg(opt, vmsg);
}

bool
isosdarwin()
{
#if defined(__APPLE__) && defined(__MACH__)
    return true;
#else
    return false;
#endif
}

std::string
envdescmsg(std::string opt)
{
    std::vector<std::string> vmsg;
    vmsg.push_back(" (env: ");
    vmsg.push_back(")");

    return envmsg(opt, vmsg);
}

std::string
envmsg(std::string opt, std::vector<std::string> &vmsg)
{
    auto v = opt2env.find(opt);
    if (v != opt2env.end()) {
        return vmsg.size() == 1 ? vmsg.at(0) + v->second
                                : pystring::join(v->second, vmsg);
    }

    return "";
}

} // namespace crosswrench

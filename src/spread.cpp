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

#include "spread.hpp"

#include "config.hpp"
#include "functions.hpp"

#include <botan/base64.h>
#include <botan/hash.h>
#include <libzippp.h>
#include <pstream.h>
#include <pystring.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

namespace crosswrench {
spread::spread(libzippp::ZipArchive &ar, bool _rootispurelib)
  : wheelfile{ ar }
  , record2write{ dotdistinfodir() + "/RECORD,," }
  , rootispurelib{ _rootispurelib }
  , destdir{ config::instance()->get_value("destdir") }
  , outmode{ std::ios_base::binary | std::ios_base::out }
{}

void
spread::compile()
{
    std::cout << "Byte-compiling .py files" << std::endl;
    std::string files;
    for (auto p : py_files) {
        std::string pyfile = p;
        files += pyfile;
        files += "\n";
    }
    const std::string cmdarg{ " -m compileall -i -" };
    std::string output;
    auto pythonint = config::instance()->get_value("python");
    auto cmd = pythonint + cmdarg;
    if (!get_cmd_output(cmd, output, files)) {
        throw std::string{ "Failed to compile .py files" };
    }
}

void
spread::install()
{
    auto files = wheelfile.getEntries();
    for (auto file : files) {
        installentry(file);
    }
    compile();
    record2write.write(rootispurelib, destdir);
}

void
spread::installdotdatadir(libzippp::ZipEntry &entry)
{
    std::vector<std::string> dirnames;
    pystring::split(entry.getName(), dirnames, "/");
    std::vector<std::string> dirnames_install{ dirnames.begin() + 2,
                                               dirnames.end() };
    installfile(entry,
                dotdatainstallpath(dirnames[1]),
                pystring::join("/", dirnames_install),
                dirnames[1] == "scripts");
}

void
spread::installentry(libzippp::ZipEntry &entry)
{
    // files that should not be installed
    if (isrecordfilenames(entry.getName()) || entry.isDirectory()) {
        return;
    }

    if (pystring::startswith(entry.getName(), dotdatadir())) {
        installdotdatadir(entry);
    }
    else {
        installfile(entry, rootinstallpath(rootispurelib), entry.getName());
    }
}

void
spread::installfile(libzippp::ZipEntry &entry,
                    std::filesystem::path prefix,
                    std::filesystem::path file,
                    bool script)
{
    bool setexec = script;
    bool replace_python = script;
    std::error_code ec;
    std::ofstream output_p;
    auto hasher = Botan::HashFunction::create("SHA-256");

    std::filesystem::path filepath = destdir;
    filepath /= prefix;
    filepath /= file;
    std::filesystem::path dirpath = filepath;
    dirpath.remove_filename();

    std::filesystem::create_directories(dirpath, ec);

    output_p.open(filepath, outmode);

    auto writer = [&](const void *data, libzippp_uint64 data_size) {
        if (replace_python) {
            auto rb = writereplacedpython(data, data_size, hasher, output_p);
            data = (const char *)data + rb;
            data_size -= rb;
            replace_python = false;
        }

        hasher->update((const uint8_t *)data, data_size);
        output_p.write((const char *)data, data_size);
        return bool(output_p);
    };

    // debugging
    std::cout << entry.getName() << " -> " << pystring::strip(filepath, "\"")
              << std::endl;

    wheelfile.readEntry(entry, writer);
    output_p.close();

    if (setexec) {
        std::filesystem::permissions(filepath,
                                     std::filesystem::perms::owner_exec |
                                       std::filesystem::perms::group_exec |
                                       std::filesystem::perms::others_exec,
                                     std::filesystem::perm_options::add);
    }

    if (pystring::endswith(entry.getName(), ".py") && !script) {
        py_files.insert(filepath);
    }

    record2write.add(entry.getName(),
                     "sha256",
                     base64urlsafenopad(Botan::base64_encode(hasher->final())),
                     std::to_string(std::filesystem::file_size(filepath)));
}

uintptr_t
spread::writereplacedpython(const void *data,
                            libzippp_uint64 data_size,
                            std::unique_ptr<Botan::HashFunction> &hasher,
                            std::ofstream &output_p)
{
    const char p_replace[] = "#!python";
    const char p_replacew[] = "#!pythonw";
    const uintptr_t w_pos = 8;
    uintptr_t replaced_bytes = 0;

    if (data_size > strlen(p_replace)) {
        if (std::memcmp(p_replace, data, std::strlen(p_replace)) == 0) {
            auto hashbangpythoninterp =
              "#!" + config::instance()->get_value("python");
            replaced_bytes = ((const char *)data)[w_pos] == 'w'
                               ? std::strlen(p_replacew)
                               : std::strlen(p_replace);

            hasher->update((const uint8_t *)hashbangpythoninterp.c_str(),
                           hashbangpythoninterp.size());
            output_p.write(hashbangpythoninterp.c_str(),
                           hashbangpythoninterp.size());
        }
    }

    return replaced_bytes;
}

} // namespace crosswrench

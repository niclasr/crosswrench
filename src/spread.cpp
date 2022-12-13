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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <botan/base64.h>
#include <botan/hash.h>
#include <libzippp.h>
#include <pstream.h>
#include <pystring.h>

#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

namespace crosswrench {
spread::spread(libzippp::ZipArchive &ar, bool _rootispurelib)
  : wheelfile{ ar }
  , record2write{ dotdistinfodir() + "/RECORD,," }
  , rootispurelib{ _rootispurelib }
  , destdir{ config::instance()->get_value("destdir") }
  , outmode{ std::ios_base::binary | std::ios_base::out }
  , verbose{ config::instance()->get_value("verbose") == "true" }
{}

void
spread::compile()
{
    std::cout << "Byte-compiling .py files" << std::endl;
    std::string files;
    for (auto p : py_files) {
        files += p.string();
        files += "\n";
    }
    const std::string cmdarg{ " -m compileall -i -" };
    std::vector<std::string> output;
    auto pythonint = config::instance()->get_value("python");
    auto cmd = pythonint + cmdarg;
    if (!get_cmd_output(cmd, output, files)) {
        throw std::string{ "Failed to compile .py files" };
    }
    if (verbose) {
        std::cout << files;
    }
}

void
spread::install()
{
    std::cout << "Installing files" << std::endl;
    auto files = wheelfile.getEntries();
    for (auto file : files) {
        // files that should not be installed
        if (isrecordfilenames(file.getName()) || file.isDirectory()) {
            continue;
        }

        installfile(file, installpath(file));
    }
    installentrypointconsolescripts();
    installinstallerfile();
    if (!config::instance()->get_value("direct-url").empty()) {
        installdirecturl();
    }
    compile();
    record2write.write(rootispurelib, destdir);
}

boost::filesystem::path
spread::createinstallpath(boost::filesystem::path prefix,
                          boost::filesystem::path end)
{
    boost::filesystem::path filepath = destdir;
    filepath /= prefix;
    filepath /= end;
    return filepath;
}

boost::filesystem::path
spread::dotdatadirinstallpath(libzippp::ZipEntry &entry)
{
    std::vector<std::string> dirnames;
    pystring::split(entry.getName(), dirnames, "/");
    std::vector<std::string> dirnames_install{ dirnames.begin() + 2,
                                               dirnames.end() };

    return createinstallpath(dotdatainstalldir(dirnames[1]),
                             pystring::join("/", dirnames_install));
}

boost::filesystem::path
spread::installpath(libzippp::ZipEntry &entry)
{
    if (pystring::startswith(entry.getName(), dotdatadir())) {
        return dotdatadirinstallpath(entry);
    }

    // root
    return createinstallpath(rootinstalldir(rootispurelib), entry.getName());
}

void
spread::installfile(libzippp::ZipEntry &entry, boost::filesystem::path filepath)
{
    if (isscript(entry)) {
        auto prefix = config::instance()->get_value("script-prefix");
        auto suffix = config::instance()->get_value("script-suffix");
        std::string newfilename = prefix + filepath.stem().string() + suffix +
                                  filepath.extension().string();
        filepath.remove_filename();
        filepath /= newfilename;
    }

    bool replace_python = isscript(entry);
    bool setexec = isscript(entry) || iselfexec(entry);

    boost::filesystem::ofstream output_p;
    auto hasher = Botan::HashFunction::create(h2b.strongest_algorithm_botan());

    createdirs(filepath);

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
    printverboseinstallloc(entry.getName(), filepath.string());

    wheelfile.readEntry(entry, writer);
    output_p.close();

    if (setexec) {
        setexecperms(filepath);
    }

    if (pystring::endswith(entry.getName(), ".py") && !isscript(entry)) {
        py_files.insert(filepath);
    }

    add2record(filepath, hasher);
}

void
spread::installfile(const char *data,
                    size_t data_size,
                    boost::filesystem::path filepath)
{
    boost::filesystem::ofstream output_p;
    auto hasher = Botan::HashFunction::create(h2b.strongest_algorithm_botan());

    createdirs(filepath);

    output_p.open(filepath, outmode);

    hasher->update((const uint8_t *)data, data_size);
    output_p.write(data, data_size);

    output_p.close();

    add2record(filepath, hasher);
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

void
spread::installinstallerfile()
{
    auto installerpath =
      destdir / rootinstalldir(rootispurelib) / dotdistinfodir() / "INSTALLER";

    std::cout << "Installing INSTALLER file" << std::endl;
    std::string installerstr = config::instance()->get_value("installer");
    installfile(installerstr.c_str(), installerstr.size(), installerpath);
    printverboseinstallloc("INSTALLER", installerpath.string());
}

void
spread::add2record(boost::filesystem::path filepath,
                   std::unique_ptr<Botan::HashFunction> &hasher)
{
    auto filepathrelroot = pystring::strip(
      boost::filesystem::relative(filepath,
                                  destdir / rootinstalldir(rootispurelib))
        .string(),
      "\"");

    record2write.add(filepathrelroot,
                     h2b.strongest_algorithm_hashlib(),
                     base64urlsafenopad(Botan::base64_encode(hasher->final())),
                     std::to_string(boost::filesystem::file_size(filepath)));
}

void
spread::createdirs(boost::filesystem::path filepath)
{
    boost::filesystem::path dirpath = filepath;
    dirpath.remove_filename();
    boost::filesystem::create_directories(dirpath);
}

void
spread::installentrypointconsolescripts()
{
    auto entrypointsfilename = dotdistinfodir() + "/entry_points.txt";
    if (wheelfile.hasEntry(entrypointsfilename)) {
        auto entry = wheelfile.getEntry(entrypointsfilename);
        auto scripts = getentrypointscripts(entry);
        if (!scripts.empty()) {
            auto scriptsdir = destdir / dotdatainstalldir("scripts");
            std::cout << "Installing entry point console scripts" << std::endl;
            for (auto script : scripts) {
                auto scriptpath = scriptsdir / script.first;
                installfile(script.second.c_str(),
                            script.second.size(),
                            scriptpath);
                setexecperms(scriptpath);
                printverboseinstallloc(script.first, scriptpath.string());
            }
        }
    }
}

void
spread::installdirecturl()
{
    std::cout << "Installing direct_url.json" << std::endl;
    boost::filesystem::ifstream input_p;
    std::ios_base::openmode inmode{ std::ios_base::binary |
                                    std::ios_base::out };

    auto directurlpath = destdir / rootinstalldir(rootispurelib) /
                         dotdistinfodir() / "direct_url.json";
    std::string url = config::instance()->get_value("direct-url");
    boost::filesystem::path archive{ config::instance()->get_value(
      "direct-url-archive") };
    std::vector<uint8_t> buf(2048);
    auto hasher = Botan::HashFunction::create(h2b.strongest_algorithm_botan());

    input_p.open(archive, inmode);

    while (input_p.good()) {
        input_p.read(reinterpret_cast<char *>(buf.data()), buf.size());
        hasher->update(buf.data(), input_p.gcount());
    }

    input_p.close();

    std::string hash =
      base64urlsafenopad(Botan::base64_encode(hasher->final()));
    std::string hash_type = h2b.strongest_algorithm_hashlib();

    std::string directurldata;
    directurldata += "{\n";
    directurldata += "    \"url\": \"" + url + "\",\n";
    directurldata += "    \"archive_info\": {\n";
    directurldata += "        \"hash\": \"" + hash_type + "=" + hash + "\"\n";
    directurldata += "    }\n";
    directurldata += "}";

    installfile(directurldata.data(), directurldata.size(), directurlpath);
    printverboseinstallloc("direct_url.json", directurlpath.string());
}

void
spread::printverboseinstallloc(std::string name, std::string loc)
{
    if (verbose) {
        std::cout << name << " -> " << loc << std::endl;
    }
}

} // namespace crosswrench

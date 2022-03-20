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

#include "record.hpp"

#include "functions.hpp"
#include "hashlib2botan.hpp"

#include <botan/base64.h>
#include <botan/hash.h>
#if defined(EXTERNAL_CSV2)
#include <csv2/reader.hpp>
#else
#include <csv2.hpp>
#endif
#include <libzippp.h>
#include <pystring.h>

#include <array>
#include <string>
#include <vector>

namespace crosswrench {

namespace {
const int RHASHTYPE = 0;
const int RHASHVALUE = 1;
const int RFILESIZE = 2;
} // namespace

record::record(std::string content)
{
    // normalize EOL:s
    std::vector<std::string> content_result;
    pystring::splitlines(content, content_result);
    content = pystring::join("\n", content_result);

    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<false>,
                 csv2::trim_policy::trim_whitespace>
      csvr;

    if (csvr.parse(content)) {
        for (const auto row : csvr) {
            std::array<std::string, 4> from_csv = { "", "", "", "" };
            std::string filepath;
            for (const auto cell : row) {
                cell.read_value(filepath);
                break;
            }
            if (filepath == (dotdistinfodir() + "/RECORD")) {
                // the RECORD file itself, special case
                from_csv[0] = filepath;
                continue;
            }
            unsigned int cell_index = 0;
            for (const auto cell : row) {
                switch (cell_index) {
                    case 0:
                        cell.read_value(from_csv[0]);
                        break;
                    case 1: {
                        std::vector<std::string> result;
                        std::string c_value;
                        hashlib2botan h2b;
                        cell.read_value(c_value);
                        pystring::partition(c_value, "=", result);
                        if (result[1].empty()) {
                            throw std::string(
                              "Record invalid, invalid syntax in hash cell");
                        }
                        std::string hashtype = pystring::lower(result[0]);
                        if (!h2b.available(hashtype)) {
                            throw std::string("RECORD uses an hash type ") +
                              hashtype + std::string(" that is not supported") +
                              std::string(" by crosswrench");
                        }
                        std::string hashvalue = result[2];
                        if (!isbase64urlsafenopad(hashvalue)) {
                            throw std::string(
                              "hash in RECORD is not a base64 encoded string");
                        }
                        from_csv[1] = hashtype;
                        from_csv[2] = hashvalue;
                        break;
                    }
                    case 2: {
                        std::string file_size;
                        cell.read_value(file_size);
                        if (!pystring::isdigit(file_size)) {
                            throw std::string("RECORD invalid, size cell do") +
                              std::string(" not only consist of digits");
                        }
                        from_csv[3] = file_size;
                        break;
                    }
                    default:
                        throw std::string(
                          "RECORD invalid, to many cells in row");
                }
                cell_index++;
            }

            if (records.count(from_csv[0]) == 1) {
                throw std::string(
                  "RECORD contains the same file multiple times");
            }
            std::array<std::string, 3> values{ from_csv[1],
                                               from_csv[2],
                                               from_csv[3] };
            records[from_csv[0]] = values;
        }
    }
    else {
        throw std::string("RECORD is empty");
    }
}

bool
record::verify(libzippp::ZipArchive &ar)
{
    std::vector<libzippp::ZipEntry> wentries = ar.getEntries();
    hashlib2botan h2b;

    for (auto i : records) {
        if (!ar.hasEntry(i.first, true)) {
            return false;
        }
    }

    for (auto we : wentries) {
        if (we.isDirectory() || we.getName() == (dotdistinfodir() + "/RECORD"))
        {
            continue;
        }

        if (records.count(we.getName()) == 0) {
            return false;
        }

        auto re = records.at(we.getName());
        auto hasher =
          Botan::HashFunction::create(h2b.hashname(re.at(RHASHTYPE)));

        auto hashupdate = [&](const void *data, libzippp_uint64 data_size) {
            hasher->update((const uint8_t *)data, data_size);
            return true;
        };

        if (ar.readEntry(we, hashupdate) != std::stoll(re.at(RFILESIZE))) {
            return false;
        }

        if (base64urlsafenopad(Botan::base64_encode(hasher->final())) !=
            re.at(RHASHVALUE)) {
            return false;
        }
    }

    return true;
}
} // namespace crosswrench

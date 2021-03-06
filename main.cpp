/* unshieldv3 -- extract InstallShield V3 archives.
Copyright (c) 2019 Wolfgang Frisch <wfrisch@riseup.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "installshieldarchivev3.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;


void usage() {
    cerr << "Usage: " << endl;
    cerr << "  unshieldv3 list ARCHIVE.Z" << endl;
    cerr << "  unshieldv3 extract ARCHIVE.Z DESTINATION" << endl;
}


int main(int argc, char** argv)
{
    std::string action;
    fs::path archive_path;
    fs::path destination_dir;

    vector<string> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(argv[i]);
    }
    if (args.size() < 3 || args[1] == "-h" || args[1] == "--help") {
        usage();
        return 1;
    }
    if (args.size() >= 3) {
        action = args[1];
        archive_path = args[2];
    }
    if (args.size() >= 4) {
        destination_dir = args[3];
    }

    if (!fs::exists(archive_path)) {
        cerr << "Archive not found: " << archive_path << endl;
        return 1;
    }
    InstallShieldArchiveV3 archive(archive_path);

    if (action == "list") {
        for (auto el : archive.files()) {
            cout << el.first << endl;
        }
    } else if (action == "extract") {
        if (destination_dir.empty()) {
            cerr << "Please specify a destination directory." << endl;
            return 1;
        }
        if (!fs::exists(destination_dir)) {
            cerr << "Destination directory not found: " << destination_dir << endl;
            return 1;
        }
        for (auto el : archive.files()) {
            const std::string& full_path = el.first;
            const InstallShieldArchiveV3::File& file = el.second;
            cout << full_path << endl;
            cout << "      Compressed size: " << setw(10) << file.compressed_size << endl;
            auto contents = archive.extract(full_path);
            cout << "    Uncompressed size: " << setw(10) << contents.size() << endl;

            string fp = full_path;
            replace(fp.begin(), fp.end(), '\\', fs::path::preferred_separator);
            fs::path dest = destination_dir / fp;
            fs::path dest_dir = dest.parent_path();
            if (!fs::create_directories(dest_dir)) {
                if (!fs::exists(dest_dir)) {
                    cerr << "Could not create directory: " << dest_dir << endl;
                    return 1;
                }
            }
            ofstream fout(dest, ios::binary | ios::out);
            if (fout.fail()) {
                cerr << dest << endl;
                cerr << "Could not create file: " << dest << endl;
                return 1;
            }
            fout.write(reinterpret_cast<char*>(contents.data()), long(contents.size()));
            if (fout.fail()) {
                cerr << "Could not write to: " << dest << endl;
                return 1;
            }
            fout.close();
        }
    } else {
        usage();
        return 1;
    }
    return 0;
}

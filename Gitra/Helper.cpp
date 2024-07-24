#include "json.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <codecvt>

using namespace std;
using namespace nlohmann;

bool exists(const string& path) {
    return filesystem::exists(path);
    /*
    ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    return ifs.good();
    */
}

string readTextFile(const string& path) {
    string line, s;
    ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    while (getline(ifs, line)) {
        s += line + "\n";
    }

    ifs.close();
    return s;
}

json jsonLoad(const string& path) {
    ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    json j = json::parse(ifs);
    ifs.close();
    return j;
}

void jsonSave(const json& j, const string& path) {
    filesystem::path filePath(path);
    filesystem::create_directories(filePath.parent_path().string());

    ofstream ofs(path.c_str(), std::ios::out | std::ios::binary);
    ofs << j;
    ofs.close();
    return;
}

wstring toWString(const string& s) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
}

bool moveFile(const string& oldPath, const string& newPath, const string& newFileName) {
    string newFullPath = newPath + newFileName;
    if (oldPath == newFullPath) return true;

    try {
        filesystem::create_directories(newPath);
        filesystem::rename(oldPath, newFullPath);
        return true;

    } catch (filesystem::filesystem_error& e) {
        cout << e.what() << "\n";
        return false;
    }
}

bool moveFolder(const string& oldPath, const string& newPath) {
    if (oldPath == newPath) return true;

    try {
        filesystem::path newFilePath(newPath);
        filesystem::create_directories(newFilePath.parent_path().string()); // Create directories all the way up to the parent directory, instead.
        filesystem::rename(oldPath, newPath);
        return true;

    }
    catch (filesystem::filesystem_error& e) {
        return false;
    }
}
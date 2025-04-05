#pragma once

#include "json.hpp"

bool exists(const std::string& path);

std::string readTextFile(const std::string& path);

nlohmann::json jsonLoad(const std::string& path);

void jsonSave(const nlohmann::json& j, const std::string& path);

std::wstring toWString(const std::string& s);

bool moveFile(const std::string& oldPath, const std::string& newPath, const std::string& newFileName);

bool moveFolder(const std::string& oldPath, const std::string& newPath);

bool recycle(const std::string& path);
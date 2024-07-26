#include "Helper.h"
#include "json.hpp"

#include <iostream>

using namespace std;
using namespace nlohmann;

// NAMES
const string programName = "Gitra";
const string versionName = "1.0.0";
const string settingsName = "settings.json";
const string checkoutName = "Checkout";
const string romName = "ROM";
const string infoFileName = "info.json";
const string saveFolderName = "Save Folder Path";

// PATHS
const string userPath = string(getenv("USERPROFILE")) + "\\";
const string documentsPath = userPath + "Documents\\";
const string repositoryPath = documentsPath + programName + "\\";
const string infoPath = repositoryPath + infoFileName;

const string localAppDataPath = userPath + "AppData\\Local\\";
const string citraName = "citra-qt.exe";
const string tempCitraPath = "binaries\\citra";
const string permCitraPath = localAppDataPath + "Citra\\nightly";
const string permCitraExePath = permCitraPath + "\\" + citraName;

const string roamingAppDataPath = string(getenv("APPDATA")) + "\\";

bool manualSubmit() {
    json info = jsonLoad(infoPath);

    // Get latest save data.
    filesystem::path saveFolderPath(roamingAppDataPath + info[saveFolderName].get<string>());
    string saveFolderFilename = saveFolderPath.filename().string();
    if (!moveFile(saveFolderPath.string(), repositoryPath, saveFolderFilename)) {
        cout << "Failed to move save data back into repository! DO NOT RUN Gitra.exe AGAIN!\n";
        return false;
    }

    // Commit changes.
    int commitResult = system(format("cd {} && git add {} {} && git commit -m Update && git push", repositoryPath, infoFileName, saveFolderFilename).c_str());

    return true;
}
int main() {
    manualSubmit();
    system("pause");
    return 0;
}
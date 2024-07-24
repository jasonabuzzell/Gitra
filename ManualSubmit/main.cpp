#include "Helper.h"
#include "json.hpp"

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

void manualSubmit() {
    // Remove checkout.
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);

    // Get latest save data.
    filesystem::path saveFolderPath(roamingAppDataPath + info[saveFolderName].get<string>());
    string saveFolderFilename = saveFolderPath.filename().string();
    moveFile(saveFolderPath.string(), repositoryPath, saveFolderFilename);

    // Commit changes.
    int commitResult = system(format("cd {} && git add {} {} && git commit -m Update && git push", repositoryPath, infoFileName, saveFolderFilename).c_str());
}
int main() {
    manualSubmit();
    return 0;
}
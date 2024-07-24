#include "json.hpp"
#include "Helper.h"

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

void resetCheckout() {
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);

    system(format("cd {} && git add {} && git commit -m \'Reset_Info\' && git push", repositoryPath, infoFileName).c_str());
}


int main() {
    resetCheckout();
    return 0;
}
#include "Helper.h"
#include "json.hpp"

using namespace std;
using namespace nlohmann;

// NAMES
const string programName = "Gitra";
const string settingsName = "settings.json";
const string checkoutName = "Checkout";
const string romName = "ROM";
const string infoFileName = "info.json";

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

void manualSubmit() {
    // Remove checkout.
    json info = jsonLoad(infoPath);
    info["Checkout"] = "";
    jsonSave(info, infoPath);

    // Zip up updated ROM.
    string romFileName = info[romName];
    int zipResult = system(format("cd {} && tar.exe -a -c -f {}.zip {}", repositoryPath, romFileName, romFileName).c_str());

    // Commit changes.
    int commitResult = system(format("cd {} && git add {} {}.zip && git commit -a -m Update && git push", repositoryPath, infoFileName, romFileName).c_str());
}
int main() {
    manualSubmit();
    return 0;
}
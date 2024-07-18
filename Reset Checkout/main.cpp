#include "json.hpp"
#include "Helper.h"

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

// This function will get the latest revision forcefully.
bool forcePull() {
    // Force pull get latest.
    if (system(format("cd {} && git reset --hard HEAD && git clean -f -d && git pull", repositoryPath).c_str())) return false;
    return true;
}

void resetCheckout() {
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);
    system(format("cd {} && git add {} && git commit -m \'Reset_Info\' && git push", repositoryPath, infoFileName).c_str());
}

int main() {
    system(format("cd {} && git stash", repositoryPath).c_str()); // Stash any changes you had prior.
    forcePull();
    resetCheckout();
    return 0;
}
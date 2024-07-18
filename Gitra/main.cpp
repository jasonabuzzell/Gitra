#include "Helper.h"
#include "json.hpp"

#include <windows.h> 
#include <stdio.h> 
#include <format>
#include <iostream>
#include <filesystem>
#include <thread>

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

// On shutdown, we should commit the repository.
void shutdown() {
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

// Intercepts close events on Windows. 
// Apparently there's a timeout of 3 seconds for close events, which makes this much more difficult to submit in time.
// Advise users to close Citra to submit, not the console.
BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
        // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: case CTRL_CLOSE_EVENT: case CTRL_BREAK_EVENT: case CTRL_LOGOFF_EVENT: case CTRL_SHUTDOWN_EVENT:
        shutdown();
        return TRUE;

    default:
        return FALSE;
    }
}

// Returns the .gitconfig username.
string getUsername() {
    string gitconfig = readTextFile(userPath + ".gitconfig");

    // Find "[user]\n\tname = " in .gitconfig
    string usernameField = "[user]\n\tname = ";
    size_t tagIndex = gitconfig.find(usernameField);
    if (tagIndex == string::npos) return "";

    // Find endIndex of value.
    size_t startIndex = tagIndex + usernameField.length();
    size_t usernameLength = gitconfig.substr(startIndex, gitconfig.length()).find("\n"); // Find the next "\n"
    if (usernameLength == string::npos) return "";

    string username = gitconfig.substr(startIndex, usernameLength);
    return username;
}

// Gets the repository.
void cloneRepository() {
    // Get repositoryURL.
    string repositoryURL;
    cout << "Please enter the repository URL: ";
    cin >> repositoryURL;
    cout << "\n";

    // Clone the repository.
    int cloneResult = system(format("git clone {} {}", repositoryURL, repositoryPath).c_str());

    // Setup repo to handle large files, if it isn't already.
    system(format("cd {} && git lfs track \"*.zip\" ", repositoryPath).c_str());
    int commitResult = system(format("cd {} && git add .gitattributes && git commit -a -m Update_Attributes && git push", repositoryPath, infoFileName).c_str());
}

bool setROM() {
    // Set and copy ROM
    json info = jsonLoad(infoPath);
    if (info[romName] == "") {

        // Update repository with the name of the ROM to run.
        string romStringPath;
        cout << "\nPlease enter the path of the .CIA/.3ds you would like to play: ";
        cin >> romStringPath;
        filesystem::path romPath(romStringPath);
        cout << "\n";

        string romFileName = romPath.filename().string();
        info[romName] = romFileName;
        jsonSave(info, infoPath);

        // Copy that ROM into the repository.
        moveFile(romStringPath, repositoryPath, romFileName);

        // Zip that ROM.
        int zipResult = system(format("cd {} && tar.exe -a -c -f {}.zip {}", repositoryPath, romFileName, romFileName).c_str());

        // Commit zipped ROM.
        int commitResult = system(format("cd {} && git add {} {}.zip && git commit -a -m Copy_ROM && git push", repositoryPath, infoFileName, romFileName).c_str());
    }

    return true;
}

// Tries to get exclusive checkout of repository.
bool getExclusiveCheckout() {
    // Check that checkout.json exists.
    if (!exists(infoPath)) {
        cout << format("Could not find checkout.json in {}!\n", repositoryPath);
        return false;
    }

    // If exists, check if anybody currently has the repo checked out.
    json info = jsonLoad(infoPath);
    if (info[checkoutName] != "") {
        cout << format("Currently checked out by {}!\n", info[checkoutName].get<string>());
        return false;
    }

    // Get username.
    int configResult = system("git config --global credential.helper store");

    // If not checked out by anybody, exclusively check it out.
    info[checkoutName] = getUsername();
    jsonSave(info, infoPath);

    string romFileName = info[romName];
    int commitResult = system(format("cd {} && git add {} && git commit -a -m Checkout && git push", repositoryPath, infoFileName).c_str());

    return true;
}

// This function will get the latest revision forcefully.
bool forcePull() {
    // Force pull get latest.
    if (system(format("cd {} && git reset --hard HEAD && git clean -f -d && git pull", repositoryPath).c_str())) return false;

    // Extract the ROM zip.
    json info = jsonLoad(infoPath);
    string romFileName = info[romName];
    if (!romFileName.empty()) {
        system(format("cd {} && tar.exe -xzvf {}.zip", repositoryPath, romFileName).c_str());
    }

    return true;
}

void clear() {
    json info;
    info[checkoutName] = "";
    info[romName] = "";
    jsonSave(info, infoPath);
    system(format("cd {} && git add {} && git commit -a -m \'Reset_Info\' && git push", repositoryPath, infoFileName).c_str());
}

void resetCheckout() {
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);
    system(format("cd {} && git add {} && git commit -m \'Reset_Info\' && git push", repositoryPath, infoFileName).c_str());
}

// MAIN
int main() {
    system(format("cd {} && git stash", repositoryPath).c_str()); // Stash any changes you had prior.

    // Set handler for closing events.
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    // INSTALL (Git)
    int gitResult = system("git --version");
    if (gitResult != 0) {
        cout << "\nGit not found! Going to run Git and Git LFS setup executables.\nPlease use the default settings when prompted.";
        int installResult = system("cd binaries && Git-2.45.2-64-bit.exe && git-lfs-windows-v3.5.1.exe");
        system("git init");
        system("git lfs install");
        cout << "\nGit installed, attempting to restart Gitra executable...";
        system("Restart.exe");
        return 0;
    }

    // SETUP (needs user input)
    if (!exists(repositoryPath)) cloneRepository();

    if (!forcePull()) {
        system("pause");
        return 2;
    }

    setROM();
    if (!exists(permCitraExePath)) moveFolder(filesystem::current_path().string() + "\\" + tempCitraPath, permCitraPath);

    // CHECKOUT (does not need user input)
    if (!getExclusiveCheckout()) {
        system("pause");
        return 3;
    }

    // RUN
    atomic<bool> done(false);
    thread citraThread([&done] {

        // Get ROM path.
        json info = jsonLoad(infoPath);
        string romPath = repositoryPath + info[romName].get<string>();

        int result = system(format("{} {}", permCitraExePath, romPath).c_str());

        done = true;

        cout << "\nClosed Citra. DO NOT CLOSE CONSOLE WINDOW! Uploading local game file to repository...";
    });

    // LOOP (keep looping until this window closes).
    while (true) {
        // Check if the other thread is still running. If not, shutdown.
        if (done) {
            shutdown();
            break;
        }

        Sleep(100); // Just to not tick so often.
    };

    return 0;
}
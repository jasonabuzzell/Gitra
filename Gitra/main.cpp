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

// HELPFUL FUNCTIONS
// git diff --cached

// NAMES
const string programName = "Gitra";
const string versionName = "1.1.2";
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

// On shutdown, we should commit the repository.
bool shutdown() {
    // Remove checkout.
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);

    // Get latest save data.
    filesystem::path saveFolderPath(roamingAppDataPath + info[saveFolderName].get<string>());
    string saveFolderFilename = saveFolderPath.filename().string();
    if (!moveFolder(saveFolderPath.string(), repositoryPath + "\\" + saveFolderFilename)) {
        cout << "Failed to move save data back into repository! DO NOT RUN Gitra.exe AGAIN!\n";
        return false;
    }

    // Commit changes.
    int commitResult = system(format("cd {} && git add {} {} && git commit -m Update && git push", repositoryPath, infoFileName, saveFolderFilename).c_str());
    cout << "Successfully committed! You may now close out of Gitra.exe.\n";

    return true;
}

// Returns the .gitconfig username.
string getUsername() {
    string gitconfig = readTextFile(userPath + ".gitconfig");

    // Find "[user]\n\tname = " in .gitconfig
    // I ASSUMED ORDER HERE! HAVE TO FIX THIS.
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

    // Add as a safe directory.
    system(format("git config --global --add safe.directory {}", repositoryPath).c_str()); // Need the repository, ugh.

    // Setup repo to handle large files, if it isn't already.
    system(format("cd {} && git lfs track \"*.3ds\" \"*.cia\"", repositoryPath).c_str());
    int commitResult = system(format("cd {} && git add .gitattributes && git commit -m Update_Attributes && git push", repositoryPath, infoFileName).c_str());
}

void setInfo() {
    json info;
    info[checkoutName] = "";
    info[romName] = "";
    info[saveFolderName] = "";
    jsonSave(info, infoPath);

    int commitResult = system(format("cd {} && git add {} && git commit -m Set_Info && git push", repositoryPath, infoFileName).c_str());
}

bool setROM() {
    // Set and copy ROM
    json info = jsonLoad(infoPath);
    if (!info.contains(romName) || info[romName] == "") {

        // Update repository with the name of the ROM to run.
        string romStringPath;
        cout << "\nPlease enter the path of the .CIA/.3ds you would like to play: ";
        getline(cin, romStringPath);
        filesystem::path romPath(romStringPath);
        cout << "\n";

        string romFileName = romPath.filename().string();
        info[romName] = romFileName;
        jsonSave(info, infoPath);

        // Copy that ROM into the repository.
        if (!moveFile(romStringPath, repositoryPath, romFileName)) {
            cout << "Failed to move ROM folder!\n";
            return false;
        }

        // Commit zipped ROM.
        int commitResult = system(format("cd {} && git add {} {} && git commit -m Copy_ROM && git push", repositoryPath, infoFileName, romFileName).c_str());
    }

    return true;
}

bool setSaveFolder() {
    // Set save folder path.
    json info = jsonLoad(infoPath);
    if (!info.contains(saveFolderName) || info[saveFolderName] == "") {

        // Update repository with the path to the save folder.
        string saveFolderPartialPath;
        cout << "\nPlease enter the path to the Save Data Folder, starting from the Roaming AppData Citra folder (e.g. Citra\\sdmc\\Nintendo 3DS\\00000000000000000000000000000000\\00000000000000000000000000000000\\title\\00040000\\00086300\\data\\00000001): ";
        getline(cin, saveFolderPartialPath);
        cout << "\n";

        info[saveFolderName] = saveFolderPartialPath;
        jsonSave(info, infoPath);

        // Copy that save folder into the directory.
        filesystem::path saveFolderPath(roamingAppDataPath + saveFolderPartialPath);
        string saveFolderFilename = saveFolderPath.filename().string();

        if (!moveFile(saveFolderPath.string(), repositoryPath, saveFolderFilename)) {
            cout << "Failed to move save folder!\n";
            return false;
        }

        // Commit zipped ROM.
        int commitResult = system(format("cd {} && git add {} {} && git commit -m Set_Save_Folder && git push", repositoryPath, infoFileName, saveFolderFilename).c_str());
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
    string username = getUsername();
    if (username.empty()) username = "Anonymous"; // Make sure we always have a name to submit with.
    info[checkoutName] = username;

    jsonSave(info, infoPath);

    string romFileName = info[romName];
    int commitResult = system(format("cd {} && git add {} && git commit -m Checkout && git push", repositoryPath, infoFileName).c_str());

    return true;
}

// This function will get the latest revision forcefully.
bool forcePull() {
    int pullResult = system(format("cd {} && git reset --hard HEAD && git clean -f && git pull", repositoryPath).c_str());
    if (pullResult != 0) return false;
    
    // Also move new save data to appropriate folder.
    json info = jsonLoad(infoPath);
    string saveFolderPartialPath = info[saveFolderName];
    filesystem::path saveFolderPath(roamingAppDataPath + saveFolderPartialPath);
    string saveFolderFilename = saveFolderPath.filename().string();
    string repositorySaveFolderPath = repositoryPath + saveFolderFilename;

    // Delete the current save folder from Citra, if exists.
    if (exists(saveFolderPath.string())) {
        int recycleResult = recycle(saveFolderPath.string());
        if (recycleResult != 0) {
            cout << "Unable to recycle prevous save folder! Going to delete instead!\n";
            filesystem::remove(saveFolderPath.string());
        }
    }

    // Move the repository save folder to the Citra folder.
    if (!moveFolder(repositorySaveFolderPath, saveFolderPath.string())) {
        cout << "Failed to move save data into the Citra folder!\n";
        return false;
    }
    
    return true;
}

void resetCheckout() {
    json info = jsonLoad(infoPath);
    info[checkoutName] = "";
    jsonSave(info, infoPath);

    system(format("cd {} && git add {} && git commit -m \'Reset_Info\' && git push", repositoryPath, infoFileName).c_str());
}

// Need to separate this out somehow.
bool installGit() {
    int gitResult = system("git --version");
    if (gitResult != 0) {
        cout << "\nGit not found! Going to run Git and Git LFS setup executables.\nPlease use the default settings when prompted.";
        int installResult = system("cd binaries && Git-2.45.2-64-bit.exe");
        cout << "\nGit installed, please restart Gitra.exe!";

        return false;
    }

    return true;
}

void installGitLFS() {
    int gitLFSResult = system("git lfs --version");
    if (gitLFSResult != 0) {
        int installResult = system("cd binaries && git-lfs-windows-v3.5.1.exe");
        system("git lfs install");
    }
}

bool setCitra() { 
    if (!moveFolder(filesystem::current_path().string() + "\\" + tempCitraPath, permCitraPath)) {
        cout << "Failed to move Citra folder into LocalAppData folder!\n";
        return false;
    }

    return true;
}

void setUsername() {
    int usernameResult = system("git config user.name");
    if (usernameResult != 0) {
        string username;
        cout << "Please set a username you'd like to use (so others can tell who is playing): ";
        cin >> username;
        int setResult = system(format("git config --global user.name {}", username).c_str());
    }
}

// MAIN
int main() {
    cout << programName + " " + versionName + "\n";
    system(format("cd {} && git stash", repositoryPath).c_str()); // Stash any changes you had prior.

    // INSTALL (Git) 
    if (!installGit()) {
        system("pause");
        return 1;
    }

    installGitLFS();

    // SETUP (needs user input)
    setUsername();

    if (!exists(repositoryPath)) cloneRepository();
    if (!forcePull()) {
        system("pause");
        return 2;
    }

    if (!exists(infoPath)) setInfo();

    if (!setROM()) {
        system("pause");
        return 3;
    }

    if (!setSaveFolder()) {
        system("pause");
        return 4;
    }

    if (!exists(permCitraExePath)) {
        if (!setCitra()) {
            system("pause");
            return 5;
        }
    }

    // CHECKOUT (does not need user input)
    if (!getExclusiveCheckout()) {
        system("pause");
        return 6;
    }

    // RUN
    atomic<bool> done(false);
    thread citraThread([&done] {

        // Get ROM path.
        json info = jsonLoad(infoPath);
        string romPath = repositoryPath + info[romName].get<string>();

        cout << "\nRunning Citra...\nDO NOT CLOSE THE CONSOLE! When finished playing, save the game, then close Citra!\n";
        int result = system(format("{} {}", permCitraExePath, romPath).c_str());

        done = true;
    });

    // LOOP (keep looping until this window closes).
    while (true) {
        // Check if the other thread is still running. If not, shutdown.
        if (done) {
            shutdown();
            break;
        }

        Sleep(100); // Just to not tick so often.
    }

    system("pause");
    return 0;
}

Welcome to Gitra!
This is a tool that will allow automatic source control of one game file with multiple people, so people can play the same game together with each other's progress.

To use Gitra, simply run the gitra-windows-1.0.0.exe (may make a macOS version if asked nicely). This will do the following:
1. Install Git and Git LFS (if necessary).
2. Ask to make a Github account (if necessary).
3. Ask for an online repository to sync a gamefile to. The repository should have a JSON file that looks like: `{ "Checkout": "", "ROM": "" }`
4. Ask for a game file (.3ds, .cia) to add to said repository.
5. Sync up said repository every time Gitra is run, to keep the user on the latest version of the game file.
6. Try to exclusively checkout the repository, so no one else can play at the same time and cause merge conflicts.
7. Run Citra with your game file.
8. When closing Citra, it will automatically upload your game file to the repository.

IMPORTANT: Do not close the console window instead of Citra when finished playing! The console needs to be open to send your latest game file.
If you forget to do this, do not run Gitra again or it will wipe the progress you just made! Instead, run ManualSubmit.exe before running Gitra,
to make sure your game file gets to the repository.

You can find binaries (.exe) included for the program on the Github page, but the code is also available for community improvement.

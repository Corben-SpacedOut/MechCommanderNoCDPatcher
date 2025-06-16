# MechCommander No-CD Patcher
A utility to patch MechCommander to remove the CD check, allowing the game to run without the original CD.
The utility creates a patched MCX-nocd.EXE in the same folder as the original MCX.EXE.

*NOTE*: This tool has only been tested on the English ISO version of MechCommander Gold found at [My Abandonware](https://www.myabandonware.com/game/mech-commander-gold-7pe). It might not work with other versions. All of the files from the CD image (which is actually a Nero Burning ROM NRG-file, not an ISO) should simply be copied to some suitable folder (like `C:\Games\MechCommander`), using e.g. [7-Zip](https://www.7-zip.org/) to read the file. Then run the utility to patch and run the newly created MCX-nocd.EXE.

## Download the executable
You can download the utility from here: [latest](https://github.com/Corben-SpacedOut/MechCommanderNoCDPatcher/releases/tag/latest)

## Building the Project
The project requires CMake to build, along with a suitable C compiler (e.g. Visual Studio with C/C++ enabled).
Either build the project as usual for a CMake project, or use the provided batch files for convenience.

* `build-n-run.bat`: Builds the debug version and runs it.
* `build-release.bat`: Builds the release version and copies the executable to the project root directory.

## Technical details
The game has nine separate functions for checking if the CD is inserted.
One of them is executed at startup, before the intro video plays. The
other eight are executed when the menu items in the main menu are clicked.

The startup CD check function is odd in that it takes an argument that explicitly enables or disables the check. Perhaps the check could be disabled by a suitable command line argument to the executable? Whatever the case, the check is readily disabled by simply falling throught the test for this argument, resulting in the function returning immediately. The function itself is located inside the executable by a sequence of bytes that make up a portion of the functions machine code.

The CD check itself is performed by scanning the drive letters from C to Z. For every CD drive found, the code checks for the existence of two files on that drive. If the files exist, the check passes and a local variable stored in a register is set to indicate a passed test and the scanning terminates. If the test fails on all drives, the code pops up a dialog prompting to insert the CD.

The utility patches the eight main menu checks by short-circuiting the two
places where the test passed flag is checked. All of these eight functions are nearly identical, so they can be located using an identical sequence of
bytes corresponding to a portion of the code. The patched instructions are also located at the same addresses relative to the code signature, so all eight functions are patched in exactly the same way.

All of the CD check functions also contain code for patching a handful of file paths to use the discovered CD drive letter, instead of whatever might have been a default. None of the file paths actually meet the criteria for patching (i.e. having a ':' in the path), so they are never modified. Maybe the original installer had some option to patch the executable to load some files from the CD instead in order to reduce the amount of disk space needed for the installation. The full 600MB or so on the CD would've taken a sizable chunk of a typical hard drive in 1999.

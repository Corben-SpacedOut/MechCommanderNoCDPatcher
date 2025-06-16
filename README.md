# MechCommander No-CD Patcher
A utility to patch MechCommander to remove the CD check, allowing the game to run without the original CD.
The utility creates a patched MCX-nocd.EXE in the same folder as the original MCX.EXE.

*NOTE*: This tool has only been tested on the English ISO version of MechCommander Gold found at [My Abandonware](https://www.myabandonware.com/game/mech-commander-gold-7pe). It might not work with other versions. All of the files from the CD image (which is actually a Nero Burning ROM NRG-file, not an ISO) should simply be copied to some suitable folder (like `C:\Games\MechCommander`), using e.g. [7-Zip](https://www.7-zip.org/) to read the file. Then run the utility to patch and run the newly created MCX-nocd.EXE. It may be useful to turn on Windows 98 compatibility mode.

## Download the executable
You can download the utility from here: [latest](https://github.com/Corben-SpacedOut/MechCommanderNoCDPatcher/releases/tag/latest)

## Building the Project
The project requires CMake to build, along with a suitable C compiler (e.g. Visual Studio with C/C++ enabled).
Either build the project as usual for a CMake project, or use the provided batch files for convenience.

* `build-n-run.bat`: Builds the debug version and runs it.
* `build-release.bat`: Builds the release version and copies the executable to the project root directory.

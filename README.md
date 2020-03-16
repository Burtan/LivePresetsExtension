# LivePresets extension
An extension for REAPER to add extended preset support

## How to build
LivePresets used gradle native as the build system. It depends on [WDL](https://github.com/justinfrankel/WDL) which is included as a git submodule. To clone the full repository use:

`git clone --recursive https://github.com/Burtan/LivePresetsExtension.git`

Build tasks to generate linux x64, windows x64 and x86 and mac x64 are included. The binaries are automatically installed into the default reaper extensions folder.

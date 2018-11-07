

## Starting from scratch (without repo)
### Create directory structure
* `src` will hold our c++ source code and header files
* commands shown below for reference only, use the commands for your system
```
cd ~
mkdir -p starcraft/MyProject/src
cd starcraft/MyProject
touch src/bot.cpp
```

### Download or compile libraries

#### Pre-compiled (windows)
https://github.com/Blizzard/s2client-api#precompiled-libs

Extract to `~/starcraft/`

Copy `SC2API_Binary_vs2017/project/maps` to `~/starcraft/MyProject/maps`

Open Visual Studio 2017.

File -> New Project from existing code -> Visual C++

* Project file location: `~/starcraft/MyProject`
* Project name: MyProjectName
* (Add subfolders checked)
* Finish

Tell project where libraries and headers are:

* Right click on MyProjectName
* Properties

Configuration: All Configurations

Platform: x64

* Configuration Properties -> General
  * Windows SDK version: (select the one you have installed)

* Configuration Properties -> VC++ Directories
  * Include Directories: edit to include: `~/starcraft/SC2API_Binary_vs2017/include`
  * Library Directories: edit to include: `~/starcraft/SC2API_Binary_vs2017/lib`
  * Apply

* Configuration Properties -> C/C++ -> Code Generation
  * For configuration release: 
    * Runtime Library: switch to /MT
  * For configuration debug:
    * Runtime Library: switch to /MTd
  * Apply

* Configuration Properties -> Linker -> Input
  * For configuration release:
    * Additional dependencies: 
    ```
    civetweb.lib
    libprotobuf.lib
    sc2api.lib
    sc2lib.lib
    sc2protocol.lib
    sc2renderer.lib
    sc2utils.lib
    SDL2.lib
    ```
  * For configuration debug:
    * Additional dependencies:
    ```
    civetweb.lib
    libprotobufd.lib
    sc2apid.lib
    sc2libd.lib
    sc2protocold.lib
    sc2rendererd.lib
    sc2utilsd.lib
    SDL2.lib
    ```
  * Ok

* Switch VS target to Debug x64
* Build -> Build Solution


#### From source (linux, mac)

Copied from: https://github.com/davechurchill/commandcenter#developer-install--compile-instructions-linux-and-os-x

```
cd ~/starcraft
git clone --recursive https://github.com/Blizzard/s2client-api && cd s2client-api
mkdir build && cd build
cmake ../
# replace 4 with number of threads on your machine
make -j4
```

```
# Assuming that you are located in the 'build' directory
# after you finished the previous step
$ cd ../

# Install SC2 API headers
sudo mkdir -p /opt/local/include \
  && sudo cp -R include/sc2api /opt/local/include \
  && sudo cp -R include/sc2renderer /opt/local/include \
  && sudo cp -R include/sc2utils /opt/local/include \
  && sudo cp -R build/generated/s2clientprotocol /opt/local/include

# Install protobuf headers
$ sudo cp -R contrib/protobuf/src/google /opt/local/include/sc2api

# Install SC2 API libraries
# (copy and paste all commands at once)
sudo mkdir -p /opt/local/lib/sc2api \
  && sudo cp build/bin/libcivetweb.a /opt/local/lib/sc2api \
  && sudo cp build/bin/libprotobuf.a /opt/local/lib/sc2api \
  && sudo cp build/bin/libsc2api.a /opt/local/lib/sc2api \
  && sudo cp build/bin/libsc2lib.a /opt/local/lib/sc2api \
  && sudo cp build/bin/libsc2protocol.a /opt/local/lib/sc2api \
  && sudo cp build/bin/libsc2utils.a /opt/local/lib/sc2api
```

This compiles on Ubuntu, but doesn't run (can't find the executable), should work on mac though?
```
cd ~/starcraft/MyProject/src
clang++ -std=c++14 -I/opt/local/include -L/opt/local/lib/sc2api bot.cpp -lsc2api -lcivetweb -lprotobuf -lsc2lib -lsc2protocol -lsc2utils -lpthread -ldl -o bot
./bot
```
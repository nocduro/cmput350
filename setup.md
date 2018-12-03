
## Setup on windows with repo
* clone repo
* download boost: https://www.boost.org/users/download/
* extract boost to convenient location
* download SC2api pre-compiled and extract: https://github.com/Blizzard/s2client-api#precompiled-libs
* create file `SC2.props` in the top level directory of the cloned repo
* modify example file below, or use visual studio GUI to generate the file. This file contains the paths to the downloaded libraries if they are not already globally available
```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <IncludePath>C:\boost_1_67_0;C:\Users\mhauc\starcraft\SC2API_Binary_vs2017\include;$(IncludePath)</IncludePath>
    <LibraryPath>C:\Users\mhauc\starcraft\SC2API_Binary_vs2017\lib;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <ItemDefinitionGroup />
  <ItemGroup />
</Project>
```
* `start sc2bot.sln`
* build

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

### Mac:
```
clang++ -std=c++14 -I/opt/local/include -L/opt/local/lib/sc2api bot.cpp -lsc2api -lcivetweb -lprotobuf -lsc2lib -lsc2protocol -lsc2utils -lpthread -ldl -framework CoreServices -o bot
```
### Other Mac Method:
You will need 3 files with cmake to make the project
```
- ~/Project/CMakeLists.txt
- ~/Project/cmake/FindSC2Api.cmake
- ~/Project/src/CMakeLists.txt
```
Create these files in the specific locations above. Copy and paste the code into the files

#### ~/Project/CMakeLists.txt
```
cmake_minimum_required(VERSION 3.1)

project(Bot)

# Specify output directories.
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

# Include custom macros to find SC2Api.
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SC2Api REQUIRED)

# Build with c++14 support, required by sc2api.
set(CMAKE_CXX_STANDARD 14)

add_subdirectory("src")
```

#### ~/Project/cmake/FindSC2Api.cmake
```
# - Try to find SC2Api
# Once done, this will define:
#
# SC2Api_FOUND - system has SC2Api.
# SC2Api_INCLUDE_DIRS - the SC2Api include directories.
# SC2Api_LIBRARIES - link these to use SC2Api.
#
# The following libraries variables are provided:
#  SC2Api_CIVETWEB_LIB, SC2Api_PROTOBUF_LIB,
#  SC2Api_SC2API_LIB, SC2Api_SC2LIB_LIB, SC2Api_SC2PROTOCOL_LIB,
#  SC2Api_SC2UTILS_LIB

# Find main Api headers.
find_path(SC2Api_INCLUDE_DIR
    NAMES
        "sc2api/sc2_api.h"
        "sc2renderer/sc2_renderer.h"
        "sc2utils/sc2_manage_process.h"
    PATHS
        "/opt/local/include"
        "/usr/local/include"
        "/usr/include"
)

# Find autogenerated Protobuf Api headers.
find_path(SC2Api_Proto_INCLUDE_DIR
    NAMES
        "s2clientprotocol/sc2api.pb.h"
    PATHS
        "/opt/local/include"
        "/usr/local/include"
        "/usr/include"
)

# Find Protobuf headers.
find_path(SC2Api_Protobuf_INCLUDE_DIR
    NAMES
        "google/protobuf/stubs/common.h"
    PATHS
        "${SC2Api_INCLUDE_DIR}/sc2api"
    NO_DEFAULT_PATH
)

# Put all the headers together.
set(SC2Api_INCLUDE_DIRS
    "${SC2Api_INCLUDE_DIR}"
    "${SC2Api_Proto_INCLUDE_DIR}"
    "${SC2Api_Protobuf_INCLUDE_DIR}"
)

set(SC2Api_LIBRARIES "")

# Search for SC2Api libraries.
foreach(COMPONENT sc2api sc2lib sc2utils sc2protocol civetweb protobuf)
    string(TOUPPER ${COMPONENT} UPPERCOMPONENT)

    find_library(SC2Api_${UPPERCOMPONENT}_LIB
        NAMES
            "${COMPONENT}"
        PATHS
            "/opt/local/lib"
            "/usr/local/lib"
        PATH_SUFFIXES
            "sc2api"
        NO_DEFAULT_PATH
    )

    if(SC2Api_${UPPERCOMPONENT}_LIB)
        mark_as_advanced(SC2Api_${UPPERCOMPONENT}_LIB)
        list(APPEND SC2Api_LIBRARIES "${SC2Api_${UPPERCOMPONENT}_LIB}")
    else()
        message(STATUS ${COMPONENT} " not found!")
        set(SC2Api_FOUND FALSE)
    endif()
endforeach()

mark_as_advanced(SC2Api_INCLUDE_DIRS SC2Api_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SC2Api DEFAULT_MSG SC2Api_INCLUDE_DIRS SC2Api_LIBRARIES)
```

#### ~/Project/src/CMakeLists.txt
```
# All the source files for the bot.
file(GLOB BOT_SOURCES "*.cpp" "*.h" "*.hpp")

# Enable compilation of the SC2 version of the bot.
add_definitions(-DSC2API)

include_directories(SYSTEM "${SC2Api_INCLUDE_DIRS}")

# Show more warnings at compiletime.
if (MSVC)
    # FIXME: put Windows specific options here.
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

# Create the executable.
add_executable(cmput350 ${BOT_SOURCES})
target_link_libraries(cmput350 ${SC2Api_LIBRARIES})

if (APPLE)
    target_link_libraries(cmput350 "-framework Carbon")
endif ()

# Linux specific.
if (UNIX AND NOT APPLE)
    target_link_libraries(cmput350 pthread dl)
endif ()
```

After all 3 files are created go to back the root directory of the project "~/Project/"

> run the command "cmake ../ -G Xcode"

> cd build/

> open "projectname".xcodeproj

After build the project and it should work




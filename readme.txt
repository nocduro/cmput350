# Running the bot (on Windows)

## Clone the repo (if needed)

## Download pre-compiled SC2API library files
* download the starcraft api library from: https://github.com/Blizzard/s2client-api#precompiled-libs
* extract somewhere outside of the repo

## Setup visual studio properties file 
* properties file tells visual studio where to find the SC2API
* create file named `SC2.props` in the root of the repository (beside sc2bot.sln)
* copy the following code and modify the path to where you extracted the precompiled libraries
```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <IncludePath>C:\SC2API_Binary_vs2017\include;$(IncludePath)</IncludePath>
    <LibraryPath>C:\SC2API_Binary_vs2017\lib;$(LibraryPath)</LibraryPath>
  </PropertyGroup>
  <ItemDefinitionGroup />
  <ItemGroup />
</Project>
```

## Open `sc2bot.sln` and build and run in release mode x64
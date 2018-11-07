
# Team Poggers - StarCraft II Bot

Group members
- Ahzam Ahmad
- Erik Ligai
- Mackenzie Hauck
- Thomas Chan


## Building instructions
Software needed:
* Starcraft II game
* Visual Studio (Windows)

### Windows
Clone repo, and follow steps in `setup.md`. Most of the configuration steps can be skipped when cloning, only the VC++ include directories need to be changed (?).

### Mac/Linux
No additional steps needed other than those in `setup.md`

### Installing maps
To run, the maps must also be copied to your systems Starcraft II installation directory under `Maps`. `CactusValleyLE.SC2Map` is included in this repo.

Example:
```
C:\Program Files (x86)\StarCraft II
├── Interfaces
│   ├── Split_1v1.SC2Interface
│   ├── Streamlined.SC2Interface
│   └── WCS_3.0.SC2Interface
├── Launcher.db
├── Maps
│   └── CactusValleyLE.SC2Map
├── SC2Data
...
```


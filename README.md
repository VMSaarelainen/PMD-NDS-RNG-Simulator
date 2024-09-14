# PMD NDS RNG Simulator

The goal of this project is to accurately simulate the random number generation and stage generation algorithms for the games Pokémon Mystery Dungeon: Red/Blue Rescue Team ("PMD1") and Pokémon Mystery Dungeon: Explorers of Time/Darkness/Sky ("PMD2").
Although attempts have been made before at reverse-engineering the algorithm, no reimplementations seem to exist which calculate the exact random number sequences the games uses. 

This is unfortunate as the games do not properly reseed RNG between parts of a stage (maze-like "floors"). 
This means if a players current in-game RNG seed can be found (e.g. with brute-forcing) the layout of the following floors can be predicted. Since exploring and fighting your way through the floors is the main gameplay loop, this tool potentially trivializes a huge part of the game and allows for superhuman gameplay.

# Faithful reimplementation

This project was heavily based on reverse-engineering the game's binaries (see [Credits](#credits)) and sometimes certain qurks of the original source must be preserved for the sake of accurate RNG simulation. 
This project aims to follows the ideas of the original games programming but with modern C++ idioms and structures.

Cleaning up the decompiled code while making sure the end result stays the same is tricky, but doable with some time. 
For example many parts of the game are written without memory reallocations, and not all of these have been rewritten using dynamic C++ containers like vectors (yet).
Additionally the source code structure (for example which control flow statements were used) is not always obvious from the decompiled code. 
Functions that suffer from this tend to be marked with a "TODO: clean up" or similar.

# What does it do

**This tool is still experimental and should not be considered accurate yet. There are also some major features waiting to be implemented.**

This tool, when given a dungeon name (only PMD2 dungeons are included by default for now), floor number and optionally an RNG seed to start from, will generate exactly the same floor as the real game would on a real console.
If no RNG seed is given, the default is 0.
For now, you will need to build the project yourself to use it (see [Building & Contributing](#building--contributing)).
This will be made easier in the future.

Since practically all of the games internals are simulated, most (documentation TBD) flags and state are preserved in the simulator. 
For example, the room ID for each tile and which of its flags are set as they are in-game. 
This information will be useful for e.g. analysis of gameplay strategies.

# Examples

Map legend:

| Tile flag | Map representation |
| -- | -- |
| End tile | S |
| Shop | K |
| Monster House | M |
| Key door | D |
| Item | o |
| Trap | X |
| Junction | + |
| Water or Lava | W |
| Chasm | C |
| Wall | * |
| Open tile | (blank) |

![Beach Cave 1F Seed 0](documentation/images/Beach%20Cave%201F%200.png)
![Beach Cave 2F Seed 1234](documentation/images/Beach%20Cave%202F%201234.png)

And for completeness, a floor that is close but not quite what the game does:

![Mt. Bristle 1F Seed 11](documentation/images/Mt.%20Bristle%201F%2011%20(bugged).png)


# Future work

This section will be updated as the work progresses, so check back later for updates!

The game actually selects one of nine algorithms to generate any given floor. Which algorithm to use is specified in that floors generation parameters. 
Most of them are fairly trivial, and they largely make use of the same functions.
All of these are a high priority feature. Currently a blank floor (all wall tiles) is returned if an unimplemented algorithm is called.
| Algorithm | Status |
| -- | -- |
| Standard | Done |
| Beetle | TBD |
| Cross | TBD |
| Crossroads | TBD |
| Line | TBD |
| One Room Monster House | TBD |
| Outer Ring | TBD |
| Outer Rooms | TBD |
| Two Rooms With Monster House | TBD |

| Features | Status | Priority |
| -- | -- | -- |
| Accurate RNG simulation | Done | High |
| All generators implemented | In progress | High |
| Basic generation functions fully working | In progress* | High |
| Spawn points and end tile simulation | TBD | High |
| Floor validity check implemented | TBD | High |
| Water/lava generation implemented | TBD | High |
| Dump PMD1 generation parameters | TBD | Mid |
| JSON output | TBD | Mid |
| Sparse JSON output | TBD | Mid |
| Documentation on tile and floor generation data | TBD | Low |
| Prebuilt release binaries | TBD | Low |
| Unit tests | TBD | Low |
| Consecutive floor generation | TBD | Low |
| Batch floor generation  | TBD | Low |
| Mission generation extras | TBD | Low |
| Unused generation features | TBD | Low |
| Fixed floor layouts | TBD | Low |
| Option for random RNG seed | TBD | Low |
| Option to print ASCII mode map legend | TBD | Low |
| Secondary (2..6) RNG generators** | TBD | Low |

\*This refers to the Generator classes member functions, except the functions that implement a separate feature. In theory there could always be edge cases or bugs, but I will mark this as done when significant testing finds no issues. 


\*\*Some details like the exact enemies and items the game spawns do have their RNG properly re-seeded. This means for humans they are effectively random, but simulating it could still be useful for TASers.

# Building & Contributing

To create a build directory and build inside it with CMake run `cmake -S . -B build` and `cmake --build build` from the project root. 
If all goes well, the executable and required JSON file will be placed in `/build/pmd_nds_dungeon_gen`.

If you're interested in the internals, also check out this projects sister repo [pmd-sky-dungeon-data-extractor](https://github.com/VMSaarelainen/pmd-sky-dungeon-data-extractor). For debugging the game in an emulator, I wrote a script for [dumping C structs from RAM](https://github.com/VMSaarelainen/FCEUX-array-of-structs-dumper). 

If you'd like to contribute, message me on Discord and I can give you some tips on getting started.

# Credits

- [pmdsky-debug](https://github.com/UsernameFodder/pmdsky-debug)
- [SkyTemple](https://github.com/SkyTemple/skytemple)


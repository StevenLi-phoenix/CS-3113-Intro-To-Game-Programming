# Project Final – In the Woods

Top-down survival action built with raylib. Fight through a procedurally populated forest, gather gold and branches, and use the compass to track down the map table where you can shop, summon the guardian, and advance to the next scene.

## Game Flow

- **Main Menu**: Play, Settings, Credits, or Quit. ENTER/SPACE starts; ESC quits.
- **Level Select**: Jump into Level 1 (Forest Grove) or Level 2 (Spreadshot Frontier with ranged attackers and richer drops).
- **Level Objective**: Collect gold (amount depends on difficulty), follow the compass to the map table, open its shop, defeat the guardian and minions, then touch the table to finish.
- **Tutorial & HUD**: Tutorial overlay auto-shows on spawn (F2 to reopen). Inventory bar shows slots and quantities; quest log tracks gold and table distance.

## Controls (default, all rebindable in F1 Settings)

- Movement: `W/A/S/D` (arrows also work).
- Use equipped item: `Z` (melee with sword, throw selected projectile, drink potion).
- Throw branch toward cursor: Left click (regardless of selection).
- Inventory slot select: Number keys `1-5` or mouse wheel.
- Open/close settings: `F1`; retry binding shown there (default ENTER).
- Reopen tutorial overlay: `F2`.
- Map table shop: Auto-opens when nearby; `Q` closes; buy items with keys `1/2/3`.
- Menu navigation: ENTER/SPACE to confirm, ESC to back out/quit.

## Gameplay Notes

- Starting kit: sword (melee), compass (points to table), throwable branches, gold counter, and limited potions.
- Upgrades: Spend gold at the map table for sword damage, shuriken throws (branches become recoverable with higher damage), and extra potions.
- Difficulty presets (F1): Easy/Normal/Hard/Impossible adjust max HP and gold required before the table will activate.
- Boss trigger: Approaching the table spawns the guardian; defeat it and any minions before touching the table to clear the level.

## Build & Run

1) Prerequisite: raylib with pkg-config available (macOS: `brew install raylib`; Linux/WSL: install raylib dev package).
2) `cd projects/project-final`
3) Build: `make` (add `DEBUG=1` for instrumentation/logging).
4) Run: `./raylib_app` or `make run`; `make debug` runs the binary with debug flag.
5) Clean: `make clean`

## Assets & Credits

- Art atlas: `assets/ElderAsset1.2.png` with UVs from `atlas_refined.json`.
- Audio: Tracks and SFX from `assets/Minifantasy_Dungeon_Music/`.
- Engine/library: raylib.

## Repository Layout (project folder)

- Core code: `main.cpp`, `leveldata/` scenes/entities/UI, `lib/` engine helpers (map streaming, navmesh, day/night, shaders, audio).
- Assets: `assets/` sprites, audio, shaders.
- Build files: `makefile`, binary `raylib_app`.

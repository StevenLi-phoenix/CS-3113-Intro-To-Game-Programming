# Project 2 - Pong

A Raylib-powered take on Pong built for CS-3113. The project started as the classic two-paddle game and grew into a sandbox packed with 100 simultaneous balls, multiple paddle control modes, and a textured scoreboard driven by a custom digits pipeline.

## Features
- 100-ball chaos mode with per-ball physics (wind resistance, friction toggles, rollback fixes on collisions).
- Keyboard, mouse-follow, and three AI difficulties for the right paddle (standard AI, nightmare lock-on, and unbeatable "god" mode).
- Textured scoreboard that reads from `assets/stacked_digits.png`/`stacked_digits.json` so digits render crisply at 72x72.
- Configurable asset pipeline scripts to remove noisy backgrounds, split digits, and rebuild the atlas used by the scoreboard.
- Cross-platform build via `make` with automatic Raylib detection through `pkg-config`.

## Controls
- `W` / `S`: Move the left paddle while in keyboard mode.
- Left mouse button + cursor: Toggle mouse-follow control for the left paddle.
- `Up` / `Down`: Move the right paddle when player-controlled.
- `T`: Switch the right paddle to assisted AI.
- `Y`: Switch the right paddle to nightmare AI (sticks to the incoming ball).
- `U`: Switch the right paddle to god mode (full-height blocker).
- `Esc`: Quit the game.

## Build & Run
1. Install Raylib and its `pkg-config` metadata (e.g. `brew install raylib` on macOS or your package manager of choice).
2. Ensure a C++11-capable compiler is on your PATH (Xcode command line tools or GCC/Clang).
3. From this directory run:
   ```bash
   make        # builds raylib_app
   make run    # launches the game
   ```
4. Use `make clean` to remove build artefacts.

The makefile automatically adds the course helper (`CS3113/cs3113.cpp`) when present and links the correct Raylib frameworks for macOS, Windows (MinGW), and Linux.

## Asset Pipeline for the Scoreboard
All asset scripts live in `assets/` and require Python 3 plus Pillow (`python3 -m pip install Pillow`). The typical pipeline for rebuilding the digit atlas is:

1. **Remove the scanned background**
   ```bash
   python assets/remove_background.py source.png assets/digits_transparent.png \
     --tolerance 28 --expand 2
   ```
   Flood-fills from the image edges to knock out noisy white backgrounds while optionally expanding the mask to catch compression artefacts.

2. **Split the cleaned sheet into normalised digits**
   ```bash
   python assets/split_digits.py assets/digits_transparent.png assets/digits_norm \
     --labels 1,2,3,4,5,6,7,8,9,0 --size 72x72
   ```
   Detects connected components by alpha, crops each digit tightly, and re-centres it on a 72x72 canvas.

3. **Stack digits into the vertical atlas and UV map**
   ```bash
   python assets/stack_digits.py assets/digits_norm assets/stacked_digits.png \
     --json assets/stacked_digits.json --labels 0,1,2,3,4,5,6,7,8,9 --size 72x72
   ```
   Generates the sprite sheet consumed by the C++ `ScoreBoard`, plus a JSON file describing per-digit UV rectangles for reference.

Adjust `--labels` or `--size` if you introduce new glyphs; the scoreboard assumes exactly ten frames stacked top-to-bottom, ordered `0` through `9`.

## Project Layout
- `main.cpp`: Game loop, physics, paddle AI, and scoreboard rendering.
- `CS3113/`: Course helper utilities (included automatically when present).
- `assets/`: Art pipeline scripts, generated digits, and the background texture.
- `makefile`: Cross-platform build rules for Raylib.

## Troubleshooting
- **Raylib not found**: Confirm `pkg-config --libs raylib` succeeds; reinstall Raylib or add its `.pc` file to `PKG_CONFIG_PATH`.
- **Python scripts complain about Pillow**: Install via `python3 -m pip install Pillow`.
- **Digits render out of order**: Regenerate the atlas with the `0-9` label order so rows align with the scoreboard's expectations.

Enjoy experimenting with the physics tweaks or swap the art by re-running the asset scripts above.

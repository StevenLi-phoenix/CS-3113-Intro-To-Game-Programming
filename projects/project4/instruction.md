# Project 4 – Rise of the AI
**NYU Tandon School of Engineering · CS-UY 3113 (Fall 2025)**  
**Due:** Saturday, Nov 8, 2025 @ 11:59 pm ET  
**No late submissions will be accepted.**

---

## Submission Instructions
1. The final project must use delta time, fixed time steps, and the Entity/Map/Scene architecture covered in class.
2. Submit the public GitHub repository link on Brightspace; commits after the deadline will be ignored.
3. Do not call OpenGL/OS APIs that we have not discussed in lecture.
4. Every `.cpp` file (including `main.cpp`) must start with the required academic honesty/header block from the syllabus.
5. Follow the standard course `main.cpp` comment template exactly as shown in earlier projects.


## Asset Requirements
You **must** provide brand-new art, music, and sound resources for this project. Anything shipped in lectures/labs (e.g., `walk.png`, `ghost.png`, `tileset.png`, witches, “Dirt Jump.wav”) is off-limits.

Recommended sources:
- [Kenney.nl](https://kenney.nl/assets) collections
- [itch.io](https://itch.io/game-assets/free) asset packs
- [OpenGameArt.org](https://opengameart.org/) for sprites, tiles, UI
- [Incompetech](https://incompetech.com/music/royalty-free/music.html) for royalty-free music

Document every asset in your README (title, author, license, URL).

---

## Requirements & Grading Breakdown

### 1. Menu Screen (10%)
- Displays the game title plus a prompt such as “Press Enter to Start”.
- Uses `KEY_ENTER` (or equivalent) to advance into gameplay.
- May use simple visuals, but it **must** be its own Scene object—not a hidden overlay.

### 2. Three Scrolling Levels (40%)
- Build three distinct platformer levels; they do **not** need to be lengthy, but each must scroll (horizontal or vertical).
- Single-screen experiences earn **zero** for the entire project.
- Remember: platformer = platforms—design traversal accordingly.

### 3. Three Lives Across the Game (20%)
- The player starts the campaign with three lives in total (not per level).
- On death, decrement lives and restart the current level.
- When lives reach zero, show “You Lose”, then return to the main menu and reset lives.
- When the player completes level three, show “You Win”, then return to the main menu and reset lives.

### 4. AI Enemies (20%)
- Implement at least three distinct AI behaviors (examples: wanderer, patroller, chaser, flying hazard).
- Contact with any AI costs the player one life and restarts the current level.
- Every level must feature at least one active AI enemy.

### 5. Audio (10%)
- Provide looping background music.
- Implement **at least three** unique sound effects (jump, damage, death, pickups, etc.).

---

## Tips
- Perform collision checks from the player’s perspective (player vs. enemy, player vs. tiles) to minimize broad-phase work.
- Expect to add helpers to `Entity.h/.cpp` for per-entity collision queries.
- Finish one level (complete loop: spawn → play → win/lose) before cloning logic for levels two and three.

---

## Extra Credit
- Build a fourth level containing a boss fight with bespoke mechanics. The boss interaction must differ meaningfully from earlier AI encounters.

---

## Final Submission Checklist
- Push all sources, assets, and build scripts required to compile/run the project on the target platform.
- Ensure the repository clearly identifies the Project 4 folder (separate from prior assignments).
- Verify that `make`, `make run`, and `make clean` succeed on a clean checkout using the documented dependencies.
- Submit the GitHub link on Brightspace before the deadline.

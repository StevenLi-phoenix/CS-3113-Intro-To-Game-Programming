# Work Plan

## Immediate gameplay TODOs
- Create Level2 scene that reuses Level1 map/player flow but swaps post-boss enemies to ATTACK1/2/3 shooters using SPREDBALL sprites, with higher coin drops.
- Add player abilities: stamina-based block that can nullify spreadball hits, sprint on Shift, and configurable dash (default X).
- Wire shop potion item and existing unified action/inputs into Level2 once the new abilities are in place.

## Level1 refactor plan (file is ~4k lines)
- Split Level1 responsibilities into focused modules/files: rendering/UI overlays (HUD, shop, tutorial, boss bar), combat/projectiles/effects, spawn/boss logic, and inventory/shop management.
- Extract spawn configs/data (trees, rocks, enemies, boss) into dedicated headers or small data structs to reduce inline constants.
- Move tutorial/help overlay logic into its own component so it can be reused (e.g., for Level2).
- Keep scene glue in Level1.cpp to orchestrate modules instead of holding all logic inline.

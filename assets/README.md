# Assets Directory

Place your game assets here:
- **textures/**: PNG/JPG sprite images
- **audio/**: WAV/OGG sound effects and background music
- **fonts/**: TTF/OTF custom typography

All contents of this `assets/` directory are automatically copied to the build output directory by CMake upon compilation.

The generated Gemini sheet in this directory has an opaque checkerboard and grid baked into it, so it is kept as the original source only. Clean, isolated sprites extracted from it are in `textures/`: `collectable_gem.png`, `snap_anchor.png`, `thorn_cluster.png`, and `moss_platform.png`. These are loaded by the resource manager and used for collectibles, glowing snap anchors, thorn hazards, and canopy platforms.

The eight transparent `textures/jungle_tree_0.png` through `textures/jungle_tree_7.png` sprites are used as layered background trees in gameplay. They are drawn behind the level geometry; parallax backgrounds remain unchanged.

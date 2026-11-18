
# Raylib Game Project

A clean C++ game development starter project using a self-contained prebuilt **Raylib** binary.

## Canopy Snap

Canopy Snap is a momentum-based jungle platformer. Swing from vines, tail-snap to anchor points, pounce through barriers, collect gems, and escape the rising spore rot. The level menu includes:

- **High Roost Run**, **Bramble Sprint**, and **Sanctum Gauntlet** for progressively tougher canopy routes.
- **Misdirection Mire**, with fragile fake anchors and slick bark.
- **Panic Pendulum**, with moving thorn logs that can be used as risky tail-snap pivots.
- **Sunk Cost Spire**, a vertical climb through falling debris.
- **Phantom Branch**, where platforms are revealed as you approach and disappear after two seconds.

Use WASD or the arrow keys to select a level, then Enter, Space, or a mouse click to play. During a level, click **MAIN MENU** in the top-right or press Esc/M to return to level selection.

Jump with Space, Up, or W. Hold Space or left mouse button near a vine to swing.

---

## 📁 Project Structure

```text
OOPs cp/
├── CMakeLists.txt        # Build script linking local lib/libraylib.a
├── README.md             # Project documentation
├── .gitignore            # Git ignore rules for build artifacts
├── .vscode/              # Editor tasks and settings
│   ├── tasks.json        # Build (Ctrl+Shift+B) and Run tasks
│   └── settings.json     # IntelliSense and build settings
├── assets/               # Place textures, audio, fonts here
├── include/              # Header files (raylib.h, raymath.h, rlgl.h)
├── lib/                  # Compiled library binary (libraylib.a)
└── src/
    └── main.cpp          # Game entry point and game loop
```

---

## 🚀 Building & Running

### Option 1: Terminal (PowerShell / Command Prompt)

1. **Configure**:
   ```powershell
   cmake -B build -G Ninja
   ```

2. **Build**:
   ```powershell
   cmake --build build
   ```

3. **Run**:
   ```powershell
   .\build\RaylibGame.exe
   ```


# Raylib Game Project

A clean C++ game development starter project using a self-contained prebuilt **Raylib** binary.

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

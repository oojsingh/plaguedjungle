#include "ResourceManager.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager() {
    UnloadAll();
}

void ResourceManager::CreateFallbackTexture() {
    if (!m_fallbackCreated) {
        Image img = GenImageChecked(32, 32, 16, 16, MAGENTA, BLACK);
        m_fallbackTexture = LoadTextureFromImage(img);
        UnloadImage(img);
        m_fallbackCreated = true;
    }
}

Texture2D& ResourceManager::LoadTexture(const std::string& name, const std::string& filepath) {
    // If already loaded, return existing
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }

    std::string resolvedPath = filepath;
    if (!FileExists(resolvedPath.c_str()) && FileExists(("../" + filepath).c_str())) {
        resolvedPath = "../" + filepath;
    }

    if (FileExists(resolvedPath.c_str())) {
        Texture2D tex = ::LoadTexture(resolvedPath.c_str());
        SetTextureFilter(tex, TEXTURE_FILTER_POINT); // Pixel-perfect rendering
        m_textures[name] = tex;
        std::cout << "[ResourceManager] Loaded texture: " << name << " from " << resolvedPath << std::endl;
        return m_textures[name];
    } else {
        std::cerr << "[ResourceManager] Warning: Texture file not found: " << filepath 
                  << ". Using fallback." << std::endl;
        CreateFallbackTexture();
        return m_fallbackTexture;
    }
}

Texture2D& ResourceManager::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    CreateFallbackTexture();
    return m_fallbackTexture;
}

bool ResourceManager::HasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

void ResourceManager::UnloadTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        ::UnloadTexture(it->second);
        m_textures.erase(it);
    }
}

void ResourceManager::UnloadAll() {
    for (auto& pair : m_textures) {
        ::UnloadTexture(pair.second);
    }
    m_textures.clear();

    if (m_fallbackCreated) {
        ::UnloadTexture(m_fallbackTexture);
        m_fallbackCreated = false;
    }
}

void ResourceManager::InitializeDefaultAssets() {
    // Ensure assets/textures directory exists
    try {
        fs::create_directories("assets/textures");
    } catch (...) {}

    // =========================================================================
    // 1. GENERATE MONKEY PLAYER SPRITESHEET (128x32: 4 frames of 32x32)
    // Frames: [0] Idle, [1] Run 1, [2] Run 2, [3] Jump
    // =========================================================================
    std::string playerPath = "assets/textures/player.png";
    if (!FileExists(playerPath.c_str()) && !FileExists(("../" + playerPath).c_str())) {
        Image img = GenImageColor(128, 32, BLANK);

        // Monkey Color Palette
        Color cFurDark      = Color{  85,  45,  15, 255 }; // Deep shadow / outline
        Color cFur          = Color{ 139,  69,  19, 255 }; // Saddle brown main fur
        Color cFurLight     = Color{ 175,  95,  30, 255 }; // Fur highlight / tufts
        Color cMuzzle       = Color{ 255, 218, 175, 255 }; // Peach face, belly, paws
        Color cMuzzleDark   = Color{ 230, 185, 140, 255 }; // Shaded muzzle
        Color cEarInner     = Color{ 240, 175, 160, 255 }; // Pinkish inner ear
        Color cEye          = Color{  20,  20,  20, 255 }; // Eye pupil
        Color cEyeShine     = Color{ 255, 255, 255, 255 }; // Eye glint
        Color cSmile        = Color{ 140,  40,  30, 255 }; // Cheerful mouth

        // --- FRAME 0: MONKEY IDLE (ox = 0) ---
        int ox = 0;
        // Tail (curled question-mark shape behind back)
        ImageDrawCircle(&img, ox + 6, 17, 2, cFur);
        ImageDrawCircle(&img, ox + 5, 14, 2, cFur);
        ImageDrawCircle(&img, ox + 6, 12, 2, cFur);
        ImageDrawCircle(&img, ox + 8, 11, 2, cFurLight);
        ImageDrawLine(&img, ox + 11, 22, ox + 7, 20, cFurDark);
        ImageDrawLine(&img, ox + 7, 20, ox + 6, 17, cFur);

        // Legs & Paws
        ImageDrawRectangle(&img, ox + 10, 24, 4, 5, cFur);
        ImageDrawRectangle(&img, ox + 17, 24, 4, 5, cFur);
        ImageDrawRectangle(&img, ox +  8, 28, 5, 3, cMuzzle); // Left foot
        ImageDrawRectangle(&img, ox + 17, 28, 5, 3, cMuzzle); // Right foot

        // Torso & Peach Belly
        ImageDrawRectangle(&img, ox + 10, 16, 11, 9, cFur);
        ImageDrawRectangle(&img, ox + 13, 17,  5, 7, cMuzzle);
        ImageDrawPixel(&img, ox + 15, 22, cMuzzleDark); // Belly button

        // Arms & Hands
        ImageDrawRectangle(&img, ox +  8, 17, 3, 7, cFur);
        ImageDrawRectangle(&img, ox +  8, 23, 3, 2, cMuzzle); // Left hand
        ImageDrawRectangle(&img, ox + 20, 17, 3, 7, cFur);
        ImageDrawRectangle(&img, ox + 20, 23, 3, 2, cMuzzle); // Right hand

        // Ears (Round cute monkey ears)
        ImageDrawCircle(&img, ox +  7, 10, 4, cFur);
        ImageDrawCircle(&img, ox +  7, 10, 2, cEarInner);
        ImageDrawCircle(&img, ox + 24, 10, 4, cFur);
        ImageDrawCircle(&img, ox + 24, 10, 2, cEarInner);

        // Head & Hair Tuft
        ImageDrawCircle(&img, ox + 15, 10, 7, cFur);
        ImageDrawRectangle(&img, ox + 14, 2, 3, 3, cFurLight); // Cute hair tuft

        // Face & Muzzle
        ImageDrawRectangle(&img, ox + 11,  8, 9, 5, cMuzzle);
        ImageDrawRectangle(&img, ox + 12, 11, 7, 4, cMuzzle);
        // Eyes
        ImageDrawRectangle(&img, ox + 12, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 12, 8, cEyeShine);
        ImageDrawRectangle(&img, ox + 17, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 17, 8, cEyeShine);
        // Nose & Smile
        ImageDrawPixel(&img, ox + 15, 12, cFurDark);
        ImageDrawLine(&img, ox + 14, 14, ox + 16, 14, cSmile);

        // --- FRAME 1: MONKEY RUN 1 (ox = 32) ---
        ox = 32;
        // Tail whipped back dynamically
        ImageDrawLine(&img, ox + 11, 21, ox + 7, 19, cFurDark);
        ImageDrawLine(&img, ox +  7, 19, ox + 4, 16, cFur);
        ImageDrawCircle(&img, ox + 3, 13, 2, cFur);
        ImageDrawCircle(&img, ox + 5, 11, 2, cFurLight);

        // Running Stride Legs
        ImageDrawRectangle(&img, ox +  8, 23, 4, 5, cFur);
        ImageDrawRectangle(&img, ox +  5, 27, 5, 3, cMuzzle); // Back foot
        ImageDrawRectangle(&img, ox + 18, 22, 4, 6, cFur);
        ImageDrawRectangle(&img, ox + 20, 27, 5, 3, cMuzzle); // Forward foot

        // Torso (leaning slightly forward)
        ImageDrawRectangle(&img, ox + 11, 15, 11, 9, cFur);
        ImageDrawRectangle(&img, ox + 14, 16,  5, 7, cMuzzle);

        // Arms in swing
        ImageDrawRectangle(&img, ox +  7, 17, 3, 6, cFur);
        ImageDrawRectangle(&img, ox +  6, 21, 3, 3, cMuzzle); // Back arm
        ImageDrawRectangle(&img, ox + 21, 15, 4, 5, cFur);
        ImageDrawRectangle(&img, ox + 23, 18, 3, 3, cMuzzle); // Forward arm

        // Ears
        ImageDrawCircle(&img, ox +  8, 10, 4, cFur);
        ImageDrawCircle(&img, ox +  8, 10, 2, cEarInner);
        ImageDrawCircle(&img, ox + 25, 10, 4, cFur);
        ImageDrawCircle(&img, ox + 25, 10, 2, cEarInner);

        // Head
        ImageDrawCircle(&img, ox + 16, 10, 7, cFur);
        ImageDrawRectangle(&img, ox + 15, 2, 3, 3, cFurLight);
        ImageDrawRectangle(&img, ox + 12,  8, 9, 5, cMuzzle);
        ImageDrawRectangle(&img, ox + 13, 11, 7, 4, cMuzzle);
        ImageDrawRectangle(&img, ox + 14, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 14, 8, cEyeShine);
        ImageDrawRectangle(&img, ox + 19, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 19, 8, cEyeShine);
        ImageDrawPixel(&img, ox + 17, 12, cFurDark);
        ImageDrawLine(&img, ox + 16, 14, ox + 18, 14, cSmile);

        // --- FRAME 2: MONKEY RUN 2 (ox = 64) ---
        ox = 64;
        // Tail raised high in motion
        ImageDrawLine(&img, ox + 10, 21, ox + 6, 18, cFurDark);
        ImageDrawCircle(&img, ox + 5, 14, 2, cFur);
        ImageDrawCircle(&img, ox + 7, 11, 2, cFurLight);

        // Opposite Stride Legs
        ImageDrawRectangle(&img, ox + 10, 22, 4, 6, cFur);
        ImageDrawRectangle(&img, ox +  9, 27, 5, 3, cMuzzle); // Front leg plant
        ImageDrawRectangle(&img, ox + 17, 23, 4, 5, cFur);
        ImageDrawRectangle(&img, ox + 21, 26, 5, 3, cMuzzle); // Back leg push

        // Torso
        ImageDrawRectangle(&img, ox + 11, 15, 11, 9, cFur);
        ImageDrawRectangle(&img, ox + 14, 16,  5, 7, cMuzzle);

        // Alternating Arms
        ImageDrawRectangle(&img, ox +  8, 16, 3, 6, cFur);
        ImageDrawRectangle(&img, ox +  8, 21, 3, 3, cMuzzle);
        ImageDrawRectangle(&img, ox + 20, 16, 3, 6, cFur);
        ImageDrawRectangle(&img, ox + 21, 21, 3, 3, cMuzzle);

        // Ears
        ImageDrawCircle(&img, ox +  8, 10, 4, cFur);
        ImageDrawCircle(&img, ox +  8, 10, 2, cEarInner);
        ImageDrawCircle(&img, ox + 25, 10, 4, cFur);
        ImageDrawCircle(&img, ox + 25, 10, 2, cEarInner);

        // Head
        ImageDrawCircle(&img, ox + 16, 10, 7, cFur);
        ImageDrawRectangle(&img, ox + 15, 2, 3, 3, cFurLight);
        ImageDrawRectangle(&img, ox + 12,  8, 9, 5, cMuzzle);
        ImageDrawRectangle(&img, ox + 13, 11, 7, 4, cMuzzle);
        ImageDrawRectangle(&img, ox + 14, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 14, 8, cEyeShine);
        ImageDrawRectangle(&img, ox + 19, 8, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 19, 8, cEyeShine);
        ImageDrawPixel(&img, ox + 17, 12, cFurDark);
        ImageDrawLine(&img, ox + 16, 14, ox + 18, 14, cSmile);

        // --- FRAME 3: MONKEY JUMP (ox = 96) ---
        ox = 96;
        // Tail streaming downwards for aerial balance
        ImageDrawLine(&img, ox + 10, 21, ox + 6, 23, cFurDark);
        ImageDrawLine(&img, ox +  6, 23, ox + 4, 26, cFur);
        ImageDrawCircle(&img, ox + 5, 28, 2, cFurLight);

        // Tucked Jump Legs
        ImageDrawRectangle(&img, ox + 10, 21, 4, 4, cFur);
        ImageDrawRectangle(&img, ox +  8, 24, 5, 3, cMuzzle);
        ImageDrawRectangle(&img, ox + 17, 21, 4, 4, cFur);
        ImageDrawRectangle(&img, ox + 17, 24, 5, 3, cMuzzle);

        // Torso
        ImageDrawRectangle(&img, ox + 11, 14, 11, 8, cFur);
        ImageDrawRectangle(&img, ox + 14, 15,  5, 6, cMuzzle);

        // Excited Arms Reaching Upwards for Vines!
        ImageDrawRectangle(&img, ox +  7, 10, 3, 6, cFur);
        ImageDrawRectangle(&img, ox +  6,  7, 4, 3, cMuzzle); // Left paw raised
        ImageDrawRectangle(&img, ox + 21, 10, 3, 6, cFur);
        ImageDrawRectangle(&img, ox + 21,  7, 4, 3, cMuzzle); // Right paw raised

        // Ears
        ImageDrawCircle(&img, ox +  8, 10, 4, cFur);
        ImageDrawCircle(&img, ox +  8, 10, 2, cEarInner);
        ImageDrawCircle(&img, ox + 25, 10, 4, cFur);
        ImageDrawCircle(&img, ox + 25, 10, 2, cEarInner);

        // Head looking up
        ImageDrawCircle(&img, ox + 16,  9, 7, cFur);
        ImageDrawRectangle(&img, ox + 15, 1, 3, 3, cFurLight);
        ImageDrawRectangle(&img, ox + 12,  7, 9, 5, cMuzzle);
        ImageDrawRectangle(&img, ox + 13, 10, 7, 4, cMuzzle);
        ImageDrawRectangle(&img, ox + 13, 7, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 13, 7, cEyeShine);
        ImageDrawRectangle(&img, ox + 18, 7, 2, 3, cEye);
        ImageDrawPixel(&img, ox + 18, 7, cEyeShine);
        ImageDrawPixel(&img, ox + 16, 11, cFurDark);
        ImageDrawRectangle(&img, ox + 15, 12, 3, 2, cSmile); // Big open smile!

        ExportImage(img, playerPath.c_str());
        UnloadImage(img);
        std::cout << "[ResourceManager] Generated Jungle Monkey spritesheet at: " << playerPath << std::endl;
    }

    // =========================================================================
    // 2. GENERATE JUNGLE TILESET (128x32: 4 tiles of 32x32)
    // Tiles: [0] Jungle Grass/Soil, [1] Mossy Temple Stone, [2] Bamboo Bridge, [3] Moving Relic Slab
    // =========================================================================
    std::string tilesetPath = "assets/textures/tileset.png";
    {
        Image img = GenImageColor(128, 32, BLANK);

        // --- TILE 0: JUNGLE TURF & RAINFOREST SOIL (ox = 0) ---
        int ox = 0;
        Color cSoil       = Color{  88,  48,  24, 255 }; // Rich dark loam
        Color cSoilDark   = Color{  62,  32,  16, 255 }; // Soil shadows
        Color cSoilLight  = Color{ 115,  68,  36, 255 }; // Soil grains
        Color cJungleTurf = Color{  34, 139,  34, 255 }; // Forest green turf
        Color cGrassLight = Color{  50, 205,  50, 255 }; // Lime sunlit blades
        Color cGrassDark  = Color{  20,  95,  25, 255 }; // Undergrowth shadow
        Color cVine       = Color{  46, 180,  65, 255 }; // Hanging vine tendrils

        // Soil base
        ImageDrawRectangle(&img, ox, 0, 32, 32, cSoil);
        for (int y = 8; y < 32; y += 3) {
            for (int x = (y % 6 == 0 ? 1 : 4); x < 32; x += 6) {
                ImageDrawPixel(&img, ox + x, y, cSoilDark);
                if (x + 2 < 32) ImageDrawPixel(&img, ox + x + 2, y + 1, cSoilLight);
            }
        }
        // Top lush grass layer
        ImageDrawRectangle(&img, ox, 0, 32, 7, cJungleTurf);
        // Grass highlights on surface
        for (int x = 0; x < 32; x += 3) {
            ImageDrawPixel(&img, ox + x, 0, cGrassLight);
            ImageDrawPixel(&img, ox + x + 1, 1, cGrassLight);
            ImageDrawPixel(&img, ox + x, 6, cGrassDark);
        }
        // Dangling jungle vines dripping into the soil
        int vineCols[] = { 3, 7, 13, 20, 27 };
        int vineLens[] = { 6, 4,  7,  5,  6 };
        for (int i = 0; i < 5; ++i) {
            for (int vy = 7; vy < 7 + vineLens[i]; ++vy) {
                ImageDrawPixel(&img, ox + vineCols[i], vy, cVine);
            }
            ImageDrawPixel(&img, ox + vineCols[i] + (i % 2 == 0 ? 1 : -1), 7 + vineLens[i] - 1, cGrassLight);
        }

        // --- TILE 1: ANCIENT MOSSY TEMPLE STONE (ox = 32) ---
        ox = 32;
        Color cStone      = Color{ 105, 115, 110, 255 }; // Weathered Mayan stone
        Color cStoneDark  = Color{  65,  75,  70, 255 }; // Mortar / carving lines
        Color cStoneLight = Color{ 145, 158, 150, 255 }; // Stone highlights
        Color cMoss       = Color{  40, 160,  60, 255 }; // Creeping jungle moss
        Color cMossLight  = Color{  75, 195,  85, 255 }; // Bright moss edges

        ImageDrawRectangle(&img, ox, 0, 32, 32, cStone);
        // Stone brick seams
        for (int y = 0; y < 32; y += 8) {
            ImageDrawLine(&img, ox, y, ox + 31, y, cStoneDark);
        }
        for (int x = 0; x < 32; x += 16) {
            ImageDrawLine(&img, ox + x, 0, ox + x, 8, cStoneDark);
            ImageDrawLine(&img, ox + x + 8, 8, ox + x + 8, 16, cStoneDark);
            ImageDrawLine(&img, ox + x, 16, ox + x, 24, cStoneDark);
            ImageDrawLine(&img, ox + x + 8, 24, ox + x + 8, 31, cStoneDark);
        }
        // Highlights on top edges of bricks
        for (int y = 1; y < 32; y += 8) {
            ImageDrawLine(&img, ox + 1, y, ox + 30, y, cStoneLight);
        }
        // Creeping lush moss over corners and crevices
        ImageDrawRectangle(&img, ox + 0, 0, 8, 4, cMoss);
        ImageDrawRectangle(&img, ox + 1, 1, 6, 2, cMossLight);
        ImageDrawRectangle(&img, ox + 22, 14, 9, 4, cMoss);
        ImageDrawRectangle(&img, ox + 23, 15, 7, 2, cMossLight);
        ImageDrawRectangle(&img, ox + 6, 23, 10, 4, cMoss);

        // --- TILE 2: JUNGLE BAMBOO & ROPE BRIDGE (ONE-WAY) (ox = 64) ---
        ox = 64;
        Color cBamboo      = Color{ 185, 175,  50, 255 }; // Bamboo yellow-green
        Color cBambooDark  = Color{ 135, 120,  30, 255 }; // Node grooves
        Color cBambooLight = Color{ 225, 215,  80, 255 }; // Sunlit cane highlight
        Color cRope        = Color{  60, 125,  50, 255 }; // Vine rope bindings
        Color cRopeDark    = Color{  40,  90,  35, 255 };

        // Main Bamboo cane log (height: 10px)
        ImageDrawRectangle(&img, ox, 0, 32, 10, cBamboo);
        ImageDrawLine(&img, ox, 1, ox + 31, 1, cBambooLight);
        ImageDrawLine(&img, ox, 9, ox + 31, 9, cBambooDark);

        // Bamboo segment nodes & tied vine lashings
        for (int bx = 2; bx < 32; bx += 10) {
            ImageDrawRectangle(&img, ox + bx, 0, 2, 10, cBambooDark);
            // Tied vine rope wraps
            ImageDrawRectangle(&img, ox + bx - 1, 2, 4, 6, cRope);
            ImageDrawPixel(&img, ox + bx, 3, cRopeDark);
            ImageDrawPixel(&img, ox + bx, 6, cRopeDark);
        }
        // Hanging jungle leaf sprouts underneath bridge
        ImageDrawPixel(&img, ox + 8,  11, cGrassLight);
        ImageDrawPixel(&img, ox + 8,  12, cJungleTurf);
        ImageDrawPixel(&img, ox + 22, 11, cGrassLight);
        ImageDrawPixel(&img, ox + 22, 12, cJungleTurf);

        // --- TILE 3: ANCIENT MOVING TEMPLE SLAB WITH EMERALD GLYPH (ox = 96) ---
        ox = 96;
        Color cRelicStone = Color{  72,  82,  80, 255 };
        Color cEmerald    = Color{   0, 255, 175, 255 }; // Glowing rune
        Color cEmeraldDim = Color{  20, 180, 125, 255 }; // Halo
        Color cGoldTrim   = Color{ 230, 180,  35, 255 };

        ImageDrawRectangle(&img, ox, 0, 32, 18, cRelicStone);
        ImageDrawRectangle(&img, ox, 0, 32, 2, cGoldTrim);
        ImageDrawRectangle(&img, ox, 16, 32, 2, cGoldTrim);

        // Carved Mayan emerald glyph in center
        ImageDrawRectangle(&img, ox + 10, 5, 12, 8, cEmeraldDim);
        ImageDrawRectangle(&img, ox + 12, 6,  8, 6, cEmerald);
        ImageDrawPixel(&img, ox + 15, 8, WHITE); // Gleam
        ImageDrawPixel(&img, ox + 16, 8, WHITE);

        // Vines clinging on sides
        ImageDrawRectangle(&img, ox + 1, 2, 3, 14, cVine);
        ImageDrawRectangle(&img, ox + 28, 2, 3, 14, cVine);

        ExportImage(img, tilesetPath.c_str());
        UnloadImage(img);
        std::cout << "[ResourceManager] Generated Jungle Tileset at: " << tilesetPath << std::endl;
    }

    // =========================================================================
    // 3. GENERATE JUNGLE ITEMS & ENEMIES (128x32: 4 frames of 32x32)
    // Frames: [0, 1] Golden Banana Collectible, [2, 3] Jungle Viper Snake Enemy
    // =========================================================================
    std::string itemsPath = "assets/textures/items.png";
    {
        Image img = GenImageColor(128, 32, BLANK);

        // --- FRAMES 0 & 1: GOLDEN BANANA CLUSTER ---
        Color cBanana      = Color{ 255, 215,   0, 255 }; // Golden ripe banana
        Color cBananaHi    = Color{ 255, 245, 140, 255 }; // Sunlit highlight
        Color cBananaShade = Color{ 210, 160,  15, 255 }; // Curve shadow
        Color cStem        = Color{  45, 135,  40, 255 }; // Fresh green stem
        Color cTip         = Color{ 100,  65,  20, 255 }; // Banana brown tips

        auto drawBananaCluster = [&](int ox, bool sparkle) {
            // Main curved central banana
            ImageDrawCircle(&img, ox + 16, 17, 7, cBanana);
            ImageDrawCircle(&img, ox + 16, 15, 6, cBananaHi);
            ImageDrawCircle(&img, ox + 16, 19, 5, cBananaShade);

            // Left banana in cluster
            ImageDrawCircle(&img, ox + 11, 18, 5, cBanana);
            ImageDrawCircle(&img, ox + 11, 16, 4, cBananaHi);

            // Right banana in cluster
            ImageDrawCircle(&img, ox + 21, 18, 5, cBanana);
            ImageDrawCircle(&img, ox + 21, 16, 4, cBananaHi);

            // Stem top
            ImageDrawRectangle(&img, ox + 14, 8, 4, 4, cStem);
            ImageDrawPixel(&img, ox + 15, 7, cTip);

            // Tips bottom
            ImageDrawPixel(&img, ox +  8, 22, cTip);
            ImageDrawPixel(&img, ox + 16, 23, cTip);
            ImageDrawPixel(&img, ox + 24, 22, cTip);

            // Sparkle stars if animated frame 1
            if (sparkle) {
                // Cross sparkle top-right
                ImageDrawPixel(&img, ox + 25,  9, WHITE);
                ImageDrawPixel(&img, ox + 24,  9, cBananaHi);
                ImageDrawPixel(&img, ox + 26,  9, cBananaHi);
                ImageDrawPixel(&img, ox + 25,  8, cBananaHi);
                ImageDrawPixel(&img, ox + 25, 10, cBananaHi);

                // Cross sparkle bottom-left
                ImageDrawPixel(&img, ox +  6, 24, WHITE);
                ImageDrawPixel(&img, ox +  5, 24, cBananaHi);
                ImageDrawPixel(&img, ox +  7, 24, cBananaHi);
            }
        };

        drawBananaCluster(0, false);
        drawBananaCluster(32, true);

        // --- FRAMES 2 & 3: JUNGLE VIPER SNAKE ENEMY ---
        Color cSnakeBody    = Color{  35, 160,  60, 255 }; // Jungle green
        Color cSnakeDiamond = Color{  80, 215, 100, 255 }; // Lime pattern
        Color cSnakeBelly   = Color{ 235, 215,  75, 255 }; // Yellow underbelly
        Color cSnakeEye     = Color{ 255, 215,   0, 255 }; // Menacing yellow eye
        Color cTongue       = Color{ 235,  40,  40, 255 }; // Flicking forked tongue

        // Frame 2: Snake Slither Pose with Tongue Out (ox = 64)
        int ox = 64;
        // Undulating body along floor
        ImageDrawRectangle(&img, ox +  4, 21, 24, 6, cSnakeBody);
        ImageDrawRectangle(&img, ox +  6, 25, 20, 2, cSnakeBelly);
        // Diamond scale patterns
        for (int dx = 7; dx < 24; dx += 5) {
            ImageDrawPixel(&img, ox + dx, 22, cSnakeDiamond);
            ImageDrawPixel(&img, ox + dx, 23, cSnakeDiamond);
        }
        // Serpent Head (raised)
        ImageDrawCircle(&img, ox + 24, 18, 5, cSnakeBody);
        ImageDrawPixel(&img, ox + 25, 17, cSnakeEye);
        ImageDrawPixel(&img, ox + 26, 17, BLACK); // Slit pupil
        // Red forked tongue flicking
        ImageDrawLine(&img, ox + 28, 19, ox + 31, 19, cTongue);
        ImageDrawPixel(&img, ox + 31, 18, cTongue);
        ImageDrawPixel(&img, ox + 31, 20, cTongue);

        // Frame 3: Snake Slither Pose with Tongue Retracted (ox = 96)
        ox = 96;
        ImageDrawRectangle(&img, ox +  3, 20, 25, 7, cSnakeBody);
        ImageDrawRectangle(&img, ox +  5, 25, 21, 2, cSnakeBelly);
        for (int dx = 6; dx < 24; dx += 5) {
            ImageDrawPixel(&img, ox + dx, 21, cSnakeDiamond);
            ImageDrawPixel(&img, ox + dx, 22, cSnakeDiamond);
        }
        ImageDrawCircle(&img, ox + 25, 19, 5, cSnakeBody);
        ImageDrawPixel(&img, ox + 26, 18, cSnakeEye);
        ImageDrawPixel(&img, ox + 27, 18, BLACK);

        ExportImage(img, itemsPath.c_str());
        UnloadImage(img);
        std::cout << "[ResourceManager] Generated Jungle Items & Enemies at: " << itemsPath << std::endl;
    }

    // Load textures into cache
    LoadTexture("player", playerPath);
    LoadTexture("tileset", tilesetPath);
    LoadTexture("items", itemsPath);

    // Load Stringstar Fields Parallax Background Layers
    LoadTexture("bg_layer_0", "assets/stringstar fields/background_0.png");
    LoadTexture("bg_layer_1", "assets/stringstar fields/background_1.png");
    LoadTexture("bg_layer_2", "assets/stringstar fields/background_2.png");
}

#pragma once

#include "Common.h"
#include <unordered_map>
#include <string>

class ResourceManager {
public:
    static ResourceManager& GetInstance();

    // Delete copy operations for Singleton pattern
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Texture management
    Texture2D& LoadTexture(const std::string& name, const std::string& filepath);
    Texture2D& GetTexture(const std::string& name);
    bool HasTexture(const std::string& name) const;
    void UnloadTexture(const std::string& name);
    void UnloadAll();

    // Generates sample spritesheets/textures into assets/textures if not present
    void InitializeDefaultAssets();

private:
    ResourceManager() = default;
    ~ResourceManager();

    std::unordered_map<std::string, Texture2D> m_textures;
    Texture2D m_fallbackTexture{};
    bool m_fallbackCreated = false;

    void CreateFallbackTexture();
};

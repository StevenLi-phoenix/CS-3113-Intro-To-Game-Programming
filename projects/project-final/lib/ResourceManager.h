#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Helper.h"

namespace ResourceKeys {
    constexpr const char *WORLD_ATLAS = "world_atlas";
}

class ResourceManager
{
public:
    static ResourceManager &instance();

    Texture2D *loadTexture(const std::string &key, const std::string &filepath);
    Texture2D *getTexture(const std::string &key);
    bool hasTexture(const std::string &key) const;

    bool loadAtlas(const std::string &textureKey, const std::string &texturePath, const char *atlasMetadataPath);

    Rectangle getSpriteRect(const std::string &tag, size_t variationIndex = 0) const;
    const std::vector<Rectangle> &getSpriteRects(const std::string &tag) const;

    void shutdown();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

    std::unordered_map<std::string, Texture2D> mTextures;
    std::unordered_map<std::string, std::vector<Rectangle>> mSpriteLookup;

    std::string readFile(const std::string &path) const;
    void parseAtlasMetadata(const std::string &contents);
    static std::string extractString(const std::string &source, const std::string &key);
    static float extractValue(const std::string &source, const std::string &key);
};

#endif

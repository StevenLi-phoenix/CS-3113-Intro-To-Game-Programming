#include "ResourceManager.h"

#include <fstream>
#include <sstream>

ResourceManager &ResourceManager::instance()
{
    static ResourceManager manager;
    return manager;
}

Texture2D *ResourceManager::loadTexture(const std::string &key, const std::string &filepath)
{
    auto it = mTextures.find(key);
    if (it != mTextures.end())
    {
        return &it->second;
    }

    Texture2D texture = LoadTexture(filepath.c_str());
    if (texture.id <= 0)
    {
        LOG_ERROR(TextFormat("Failed to load texture: %s", filepath.c_str()));
        return nullptr;
    }

    mTextures.emplace(key, texture);
    return &mTextures[key];
}

Texture2D *ResourceManager::getTexture(const std::string &key)
{
    auto it = mTextures.find(key);
    if (it == mTextures.end()) return nullptr;
    return &it->second;
}

bool ResourceManager::hasTexture(const std::string &key) const
{
    return mTextures.find(key) != mTextures.end();
}

bool ResourceManager::loadAtlas(const std::string &textureKey, const std::string &texturePath, const std::string &atlasMetadataPath)
{
    Texture2D *atlasTexture = loadTexture(textureKey, texturePath);
    if (!atlasTexture)
    {
        return false;
    }

    std::string contents = readFile(atlasMetadataPath);
    if (contents.empty())
    {
        LOG_ERROR(TextFormat("Failed to read atlas metadata: %s", atlasMetadataPath.c_str()));
        return false;
    }

    parseAtlasMetadata(contents);
    return true;
}

Rectangle ResourceManager::getSpriteRect(const std::string &tag, size_t variationIndex) const
{
    const auto &rects = getSpriteRects(tag);
    if (rects.empty())
    {
        return {0, 0, 0, 0};
    }

    size_t index = variationIndex % rects.size();
    return rects[index];
}

const std::vector<Rectangle> &ResourceManager::getSpriteRects(const std::string &tag) const
{
    static const std::vector<Rectangle> EMPTY;
    auto it = mSpriteLookup.find(tag);
    if (it == mSpriteLookup.end())
    {
        return EMPTY;
    }
    return it->second;
}

void ResourceManager::shutdown()
{
    for (auto &entry : mTextures)
    {
        if (entry.second.id > 0)
        {
            UnloadTexture(entry.second);
        }
    }
    mTextures.clear();
    mSpriteLookup.clear();
}

std::string ResourceManager::readFile(const std::string &path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return {};
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

void ResourceManager::parseAtlasMetadata(const std::string &contents)
{
    size_t cursor = 0;
    while (true)
    {
        size_t tagPos = contents.find("\"tag\"", cursor);
        if (tagPos == std::string::npos) break;

        size_t objStart = contents.rfind("{", tagPos);
        size_t objEnd = contents.find("}", tagPos);
        if (objStart == std::string::npos || objEnd == std::string::npos) break;

        std::string object = contents.substr(objStart, objEnd - objStart);
        std::string tag = extractString(object, "tag");
        float x = extractValue(object, "x");
        float y = extractValue(object, "y");
        float w = extractValue(object, "w");
        float h = extractValue(object, "h");

        if (!tag.empty() && w > 0.0f && h > 0.0f)
        {
            Rectangle rect = {x, y, w, h};
            mSpriteLookup[tag].push_back(rect);
        }

        cursor = objEnd;
    }
}

std::string ResourceManager::extractString(const std::string &source, const std::string &key)
{
    std::string pattern = "\"" + key + "\"";
    size_t keyPos = source.find(pattern);
    if (keyPos == std::string::npos) return {};
    size_t firstQuote = source.find('"', keyPos + pattern.size());
    if (firstQuote == std::string::npos) return {};
    size_t secondQuote = source.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos) return {};
    return source.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

float ResourceManager::extractValue(const std::string &source, const std::string &key)
{
    std::string pattern = "\"" + key + "\"";
    size_t keyPos = source.find(pattern);
    if (keyPos == std::string::npos) return 0.0f;
    size_t colonPos = source.find(':', keyPos + pattern.size());
    if (colonPos == std::string::npos) return 0.0f;
    size_t endPos = source.find_first_of(",}", colonPos + 1);
    std::string valueStr = source.substr(colonPos + 1, endPos - colonPos - 1);
    try
    {
        return std::stof(valueStr);
    }
    catch (...)
    {
        return 0.0f;
    }
}

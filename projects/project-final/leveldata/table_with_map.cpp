#include "table_with_map.h"

#include "../lib/ResourceManager.h"
#include "ResourceTags.h"

namespace
{
    constexpr const char *TABLE_TAG = tags::TABLEWITHMAP;
    constexpr Vector2 DEFAULT_SCALE = {120.0f, 78.0f};
}

TableWithMap::TableWithMap()
    : Entity()
{
    setScale(DEFAULT_SCALE);
    setColliderDimensions({DEFAULT_SCALE.x, DEFAULT_SCALE.y * 0.35f});
    configureSprite();
    setIsActive(true);
    setCanCollide(true);
}

void TableWithMap::configureSprite()
{
    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlas = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    Rectangle rect = rm.getSpriteRect(TABLE_TAG);
    if (atlas && rect.width > 0.0f && rect.height > 0.0f)
    {
        setTexture(*atlas);
        setOwnsTexture(false);
        setCustomSourceRect(rect);
    }
}

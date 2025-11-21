#include "player.h"
#include "../lib/ResourceManager.h"

Player::Player(Vector2 position, Vector2 scale)
    : Entity()
{
    setPosition(position);
    setScale(scale);
    setColliderDimensions(scale);

    ResourceManager &rm = ResourceManager::instance();
    Texture2D *atlasTexture = rm.getTexture(ResourceKeys::WORLD_ATLAS);
    Rectangle spriteRect = rm.getSpriteRect(PlayerConstants::SPRITE_TAG);

    if (atlasTexture && spriteRect.width > 0.0f && spriteRect.height > 0.0f)
    {
        setTexture(*atlasTexture);
        setOwnsTexture(false);
        setCustomSourceRect(spriteRect);
    }
    else if (atlasTexture)
    {
        setTexture(*atlasTexture);
        setOwnsTexture(false);
    }
    else
    {
        Texture2D fallback = LoadTexture(PlayerConstants::FALLBACK_TEXTURE_PATH);
        if (fallback.id > 0)
        {
            setTexture(fallback);
            setOwnsTexture(true);
        }
    }
}

Player::~Player()
{
}

void Player::moveLeft()
{
    Vector2 movement = getMovement();
    movement.x = -15.0f;
    setMovement(movement);
}

void Player::moveRight()
{
    Vector2 movement = getMovement();
    movement.x = 15.0f;
    setMovement(movement);
}

void Player::moveUp()
{
    Vector2 movement = getMovement();
    movement.y = -15.0f;
    setMovement(movement);
}

void Player::moveDown()
{
    Vector2 movement = getMovement();
    movement.y = 15.0f;
    setMovement(movement);
}

#include "player.h"

Player::Player(Vector2 position, Vector2 scale, const char *texturePath) : Entity(position, scale, texturePath)
{
}

Player::~Player()
{
}

void Player::moveLeft()
{
    setMovement({-1.0f, 0.0f});
}

void Player::moveRight()
{
    setMovement({1.0f, 0.0f});
}

void Player::moveUp()
{
    setMovement({0.0f, -1.0f});
}

void Player::moveDown()
{
    setMovement({0.0f, 1.0f});
}
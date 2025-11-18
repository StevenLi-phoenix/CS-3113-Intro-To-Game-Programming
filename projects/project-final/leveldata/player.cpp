#include "player.h"

Player::Player(Vector2 position, Vector2 scale, const char *texturePath) : Entity(position, scale, texturePath)
{
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

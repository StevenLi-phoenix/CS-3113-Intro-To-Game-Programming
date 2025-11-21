#include "../lib/Entity.h"
#include "../constants.h"
#include <string>

namespace PlayerConstants {
    constexpr const char *SPRITE_TAG = "PLAYER";
    constexpr const char *FALLBACK_TEXTURE_PATH = "assets/player.png";
}

class Player : public Entity
{
public:
    Player(Vector2 position = c::ORIGIN, Vector2 scale = {180.0f, 250.0f});
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    ~Player();
};
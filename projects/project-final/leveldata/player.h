#include "../lib/Entity.h"
#include "../constants.h"
#include <string>

namespace PlayerConstants {
    constexpr const char *TEXTURE_PATH = "assets/player.png";
}

class Player : public Entity
{
private:
    const char *mTexturePath;
public:
    Player(Vector2 position = c::ORIGIN, Vector2 scale = {100.0f, 100.0f}, const char *texturePath = PlayerConstants::TEXTURE_PATH);
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    ~Player();
};
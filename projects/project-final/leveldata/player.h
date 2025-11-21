#ifndef PLAYER_H
#define PLAYER_H

#include "../lib/Entity.h"
#include "../constants.h"
#include <string>

namespace PlayerConstants {
    constexpr const char *SPRITE_TAG = "PLAYER";
    constexpr const char *FALLBACK_TEXTURE_PATH = "assets/player.png";
    constexpr float MAX_HEALTH = 10.0f;
    constexpr float DAMAGE_COOLDOWN_SECONDS = 0.8f;
}

class Player : public Entity
{
public:
    Player(Vector2 position = c::ORIGIN, Vector2 scale = {180.0f, 250.0f});

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;

    float getHealth() const { return mHealth; }
    float getMaxHealth() const { return mMaxHealth; }
    bool isDead() const { return mHealth <= 0.0f; }

    bool applyDamage(float amount);
    void heal(float amount);
    void restoreFullHealth();
    void setMaxHealth(float maxHealth, bool refill = true);

    ~Player();

private:
    float mMaxHealth = PlayerConstants::MAX_HEALTH;
    float mHealth = PlayerConstants::MAX_HEALTH;
    float mDamageCooldownTimer = 0.0f;
};

#endif // PLAYER_H
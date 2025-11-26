#include "player.h"
#include "../lib/ResourceManager.h"
#include "../lib/Music.h"
#include <algorithm>

namespace
{
    constexpr const char *SFX_PLAYER_HURT[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/11_human_damage_1.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/11_human_damage_2.wav",
        "assets/Minifantasy_Dungeon_Music/SFX/11_human_damage_3.wav"
    };
    constexpr size_t SFX_PLAYER_HURT_COUNT = sizeof(SFX_PLAYER_HURT) / sizeof(SFX_PLAYER_HURT[0]);

    constexpr const char *SFX_PLAYER_DEATH[] = {
        "assets/Minifantasy_Dungeon_Music/SFX/14_human_death_spin.wav"
    };
    constexpr size_t SFX_PLAYER_DEATH_COUNT = sizeof(SFX_PLAYER_DEATH) / sizeof(SFX_PLAYER_DEATH[0]);

    void playRandomSFX(const char* const* options, size_t count)
    {
        if (!options || count == 0)
        {
            return;
        }
        const int index = GetRandomValue(0, static_cast<int>(count) - 1);
        AudioManager::playSFX(options[index]);
    }
}

Player::Player(Vector2 position, Vector2 scale)
    : Entity(),
      mMaxHealth(PlayerConstants::MAX_HEALTH),
      mHealth(PlayerConstants::MAX_HEALTH),
      mDamageCooldownTimer(0.0f)
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

    setTextureFacesLeft(false);
    setIsPushable(true);
}

Player::~Player()
{
}

void Player::moveLeft()
{
    Vector2 movement = getMovement();
    movement.x = -5.0f;
    setMovement(movement);
    setIsHorizontalFlipped(true);
}

void Player::moveRight()
{
    Vector2 movement = getMovement();
    movement.x = 5.0f;
    setMovement(movement);
    setIsHorizontalFlipped(false);
}

void Player::moveUp()
{
    Vector2 movement = getMovement();
    movement.y = -5.0f;
    setMovement(movement);
}

void Player::moveDown()
{
    Vector2 movement = getMovement();
    movement.y = 5.0f;
    setMovement(movement);
}

void Player::update(float deltaTime,
                    Entity *player,
                    Map *map,
                    const std::vector<Entity*> &collidableEntities)
{
    if (!getIsActive())
    {
        return;
    }

    if (mDamageCooldownTimer > 0.0f)
    {
        mDamageCooldownTimer = std::max(0.0f, mDamageCooldownTimer - deltaTime);
    }

    Entity::update(deltaTime, player, map, collidableEntities);
}

bool Player::applyDamage(float amount)
{
    if (amount <= 0.0f || !getIsActive())
    {
        return false;
    }

    if (mDamageCooldownTimer > 0.0f)
    {
        return false;
    }

    const float previousHealth = mHealth;
    mHealth = std::max(0.0f, mHealth - amount);
    mDamageCooldownTimer = PlayerConstants::DAMAGE_COOLDOWN_SECONDS;

    if (mHealth <= 0.0f)
    {
        setIsActive(false);
        setCanCollide(false);
        LOG_INFO("Player defeated");
        playRandomSFX(SFX_PLAYER_DEATH, SFX_PLAYER_DEATH_COUNT);
    }
    else if (mHealth < previousHealth)
    {
        playRandomSFX(SFX_PLAYER_HURT, SFX_PLAYER_HURT_COUNT);
    }

    return true;
}

void Player::heal(float amount)
{
    if (amount <= 0.0f)
    {
        return;
    }

    mHealth = std::clamp(mHealth + amount, 0.0f, mMaxHealth);
}

void Player::restoreFullHealth()
{
    mHealth = mMaxHealth;
    mDamageCooldownTimer = 0.0f;
    setIsActive(true);
    setCanCollide(true);
}

void Player::setMaxHealth(float maxHealth, bool refill)
{
    mMaxHealth = std::max(1.0f, maxHealth);
    if (refill)
    {
        mHealth = mMaxHealth;
        mDamageCooldownTimer = 0.0f;
        setIsActive(true);
        setCanCollide(true);
    }
    else
    {
        mHealth = std::clamp(mHealth, 0.0f, mMaxHealth);
        if (mHealth <= 0.0f)
        {
            setIsActive(false);
            setCanCollide(false);
        }
    }
}

#include "ProgressBar.h"

ProgressBar::ProgressBar()
    : mMinValue(0.0f), mMaxValue(1.0f), mValue(0.5f),
      mBackgroundColor(Fade(LIGHTGRAY, 0.5f)), mFillColor(GREEN),
      mBorderColor(DARKGRAY), mBorderThickness(2.0f)
{
    setIsActive(true);
    setCanCollide(false);
    setScale({200.0f, 20.0f});
}

ProgressBar::ProgressBar(Vector2 position, Vector2 size)
    : ProgressBar()
{
    setPosition(position);
    setScale(size);
}

void ProgressBar::setRange(float minValue, float maxValue)
{
    if (minValue == maxValue)
    {
        maxValue = minValue + 1.0f;
    }
    mMinValue = std::min(minValue, maxValue);
    mMaxValue = std::max(minValue, maxValue);
    setValue(mValue);
}

void ProgressBar::setValue(float value)
{
    if (mMaxValue <= mMinValue)
    {
        mValue = mMinValue;
        return;
    }
    mValue = Clamp(value, mMinValue, mMaxValue);
}

void ProgressBar::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;
}

void ProgressBar::render()
{
    if (!getIsActive()) return;

    Vector2 position = getPosition();
    Vector2 size = getScale();

    Rectangle backgroundRect = {
        position.x - size.x / 2.0f,
        position.y - size.y / 2.0f,
        size.x,
        size.y
    };

    DrawFilledRectangle(backgroundRect, mBackgroundColor);
    DrawRectangleBorder(backgroundRect, mBorderThickness, mBorderColor);

    float normalized = (mMaxValue <= mMinValue)
        ? 0.0f
        : (mValue - mMinValue) / (mMaxValue - mMinValue);

    Rectangle fillRect = backgroundRect;
    fillRect.width = backgroundRect.width * normalized;
    DrawFilledRectangle(fillRect, mFillColor);
}

void ProgressBar::shutdown()
{
    Entity::shutdown();
}

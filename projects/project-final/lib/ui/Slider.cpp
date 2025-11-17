#include "Slider.h"
#include <algorithm>
#include <cmath>
#include <limits>

Slider* Slider::sCurrentHoveredSlider = nullptr;

Slider::Slider()
    : mMinValue(0.0f), mMaxValue(1.0f), mValue(0.0f), mIsDragging(false),
      mTrackColor(LIGHTGRAY), mFillColor(DARKGRAY), mKnobColor(RAYWHITE),
      mBorderColor(BLACK), mBorderThickness(2.0f), mOnValueChanged(nullptr),
      mSnapEnabled(false), mSnapTolerance(0.0f), mSnapValues(),
      mIsSnapped(false), mSnappedValue(0.0f)
{
    setIsActive(true);
    setCanCollide(false);
    setScale({200.0f, 16.0f});
}

Slider::Slider(Vector2 position, Vector2 size, float minValue, float maxValue, float startValue)
    : mMinValue(minValue), mMaxValue(maxValue), mValue(startValue), mIsDragging(false),
      mTrackColor(LIGHTGRAY), mFillColor(DARKGRAY), mKnobColor(RAYWHITE),
      mBorderColor(BLACK), mBorderThickness(2.0f), mOnValueChanged(nullptr),
      mSnapEnabled(false), mSnapTolerance(0.0f), mSnapValues(),
      mIsSnapped(false), mSnappedValue(0.0f)
{
    if (mMinValue == mMaxValue)
    {
        mMaxValue = mMinValue + 1.0f;
    }
    setPosition(position);
    setScale(size);
    setIsActive(true);
    setCanCollide(false);
    setValue(mValue, false);
}

void Slider::setRange(float minValue, float maxValue)
{
    if (minValue == maxValue)
    {
        maxValue = minValue + 1.0f;
    }
    mMinValue = std::min(minValue, maxValue);
    mMaxValue = std::max(minValue, maxValue);
    setValue(mValue, false);
}

void Slider::setValue(float value, bool fireCallback)
{
    float clamped = value;
    if (mMaxValue > mMinValue)
    {
        clamped = Clamp(value, mMinValue, mMaxValue);
    }
    float snappedValue = clamped;
    bool snapped = getSnappedValue(clamped, snappedValue);
    if (snapped)
    {
        clamped = snappedValue;
        mIsSnapped = true;
        mSnappedValue = snappedValue;
    }
    else
    {
        mIsSnapped = false;
        mSnappedValue = clamped;
    }

    if (fabsf(clamped - mValue) < 0.0001f)
    {
        mValue = clamped;
        return;
    }

    mValue = clamped;
    if (mOnValueChanged && fireCallback)
    {
        mOnValueChanged(mValue);
    }
}

Rectangle Slider::getTrackRectangle() const
{
    Vector2 position = getPosition();
    Vector2 size = getScale();
    return {
        position.x - size.x / 2.0f,
        position.y - size.y / 2.0f,
        size.x,
        size.y
    };
}

float Slider::getNormalizedValue() const
{
    if (mMaxValue <= mMinValue)
    {
        return 0.0f;
    }
    return (mValue - mMinValue) / (mMaxValue - mMinValue);
}

bool Slider::isMouseOver() const
{
    return UIBase::isMouseOver();
}

void Slider::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive()) return;

    bool hover = isMouseOver();
    if (hover)
    {
        sCurrentHoveredSlider = this;
    }
    else if (sCurrentHoveredSlider == this)
    {
        sCurrentHoveredSlider = nullptr;
    }

    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        mIsDragging = true;
    }

    if (mIsDragging)
    {
        Rectangle track = getTrackRectangle();
        Vector2 mousePos = GetMousePosition();
        float normalized = 0.0f;
        if (track.width > 0.0f)
        {
            normalized = Clamp((mousePos.x - track.x) / track.width, 0.0f, 1.0f);
        }
        float newValue = mMinValue + normalized * (mMaxValue - mMinValue);
        setValue(newValue);

        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            mIsDragging = false;
        }
    }
}

void Slider::render()
{
    if (!getIsActive()) return;

    Rectangle track = getTrackRectangle();
    float normalized = getNormalizedValue();

    DrawFilledRectangle(track, mTrackColor);
    DrawRectangleBorder(track, mBorderThickness, mBorderColor);

    Rectangle fillRect = track;
    fillRect.width = track.width * normalized;
    DrawFilledRectangle(fillRect, mFillColor);

    float knobSize = track.height * 1.5f;
    if (knobSize < track.height)
    {
        knobSize = track.height;
    }
    float knobX = track.x + normalized * track.width;
    Rectangle knob = {
        knobX - knobSize / 2.0f,
        track.y + track.height / 2.0f - knobSize / 2.0f,
        knobSize,
        knobSize
    };

    if (mSnapEnabled && !mSnapValues.empty())
    {
        float tickWidth = track.width * 0.02f;
        tickWidth = Clamp(tickWidth, track.height * 0.3f, track.width * 0.06f);
        float tickHeight = track.height * 0.5f;

        for (float snapValue : mSnapValues)
        {
            float normalizedSnap = 0.0f;
            if (mMaxValue > mMinValue)
            {
                normalizedSnap = (snapValue - mMinValue) / (mMaxValue - mMinValue);
            }
            normalizedSnap = Clamp(normalizedSnap, 0.0f, 1.0f);

            float snapX = track.x + normalizedSnap * track.width;
            Rectangle snapHint = {
                snapX - tickWidth / 2.0f,
                track.y + track.height / 2.0f - tickHeight / 2.0f,
                tickWidth,
                tickHeight
            };

            bool isCurrentSnap = mIsSnapped && fabsf(snapValue - mSnappedValue) < 0.0001f;
            Color fillColor = isCurrentSnap ? Fade(GRAY, 0.8f) : Fade(GRAY, 0.3f);
            Color borderColor = isCurrentSnap ? Fade(DARKGRAY, 0.9f) : Fade(DARKGRAY, 0.4f);

            DrawFilledRectangle(snapHint, fillColor);
            DrawRectangleBorder(snapHint, 1.0f, borderColor);
        }
    }

    Color knobColor = mKnobColor;
    if (mIsDragging)
    {
        knobColor = AdjustColorBrightness(knobColor, -0.2f);
    }
    else if (isMouseOver())
    {
        knobColor = AdjustColorBrightness(knobColor, 0.1f);
    }

    DrawFilledRectangle(knob, knobColor);
    DrawRectangleBorder(knob, 2.0f, mBorderColor);
}

void Slider::updateGlobalCursor()
{
    if (sCurrentHoveredSlider != nullptr && (sCurrentHoveredSlider->isMouseOver() || sCurrentHoveredSlider->isDragging()))
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        sCurrentHoveredSlider = nullptr;
    }
}

void Slider::shutdown()
{
    Entity::shutdown();
}

void Slider::setSnapValues(const std::vector<float>& values)
{
    mSnapValues = values;
    std::sort(mSnapValues.begin(), mSnapValues.end());
    mSnapValues.erase(
        std::unique(
            mSnapValues.begin(),
            mSnapValues.end(),
            [](float a, float b) { return fabsf(a - b) < 0.0001f; }),
        mSnapValues.end());
}

void Slider::addSnapValue(float value)
{
    mSnapValues.push_back(value);
    std::sort(mSnapValues.begin(), mSnapValues.end());
    mSnapValues.erase(
        std::unique(
            mSnapValues.begin(),
            mSnapValues.end(),
            [](float a, float b) { return fabsf(a - b) < 0.0001f; }),
        mSnapValues.end());
}

bool Slider::getSnappedValue(float candidate, float &outValue) const
{
    if (!mSnapEnabled || mSnapValues.empty())
    {
        return false;
    }

    float closestDiff = std::numeric_limits<float>::max();
    float closestValue = candidate;
    for (float snapValue : mSnapValues)
    {
        float diff = fabsf(candidate - snapValue);
        if (diff < closestDiff)
        {
            closestDiff = diff;
            closestValue = snapValue;
        }
    }

    if (mSnapTolerance <= 0.0f || closestDiff <= mSnapTolerance)
    {
        outValue = closestValue;
        return true;
    }

    return false;
}

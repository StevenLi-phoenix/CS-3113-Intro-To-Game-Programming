#include "DayNightCycle.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float HALF_PI = 1.57079632679f;
}

DayNightCycle::DayNightCycle(float secondsPerDay)
    : mSecondsPerDay(std::max(secondsPerDay, 1.0f)),
      mCurrentTime(0.35f * std::max(secondsPerDay, 1.0f)),
      mTimeScale(1.0f),
      mPaused(false)
{
}

void DayNightCycle::update(float deltaTime)
{
    if (mPaused || deltaTime <= 0.0f)
    {
        return;
    }

    mCurrentTime += deltaTime * mTimeScale;

    if (mCurrentTime >= mSecondsPerDay || mCurrentTime < 0.0f)
    {
        mCurrentTime = std::fmod(mCurrentTime, mSecondsPerDay);
        if (mCurrentTime < 0.0f)
        {
            mCurrentTime += mSecondsPerDay;
        }
    }
}

void DayNightCycle::setTimeScale(float scale)
{
    mTimeScale = scale;
}

void DayNightCycle::setNormalizedTime(float normalizedTime)
{
    const float clamped = std::clamp(normalizedTime, 0.0f, 1.0f);
    mCurrentTime = clamped * mSecondsPerDay;
}

void DayNightCycle::setPaused(bool paused)
{
    mPaused = paused;
}

float DayNightCycle::getNormalizedTime() const
{
    if (mSecondsPerDay <= 0.0f)
    {
        return 0.0f;
    }

    float normalized = mCurrentTime / mSecondsPerDay;
    if (normalized < 0.0f)
    {
        normalized += 1.0f;
    }
    else if (normalized >= 1.0f)
    {
        normalized -= std::floor(normalized);
    }
    return normalized;
}

float DayNightCycle::getAmbientIntensity() const
{
    const float normalized = getNormalizedTime();
    const float sunCurve = 0.5f + 0.5f * std::sinf((normalized * TWO_PI) - HALF_PI);
    const float lerp = std::clamp(sunCurve, 0.0f, 1.0f);
    return lighting::MIN_AMBIENT_INTENSITY +
           (lighting::MAX_AMBIENT_INTENSITY - lighting::MIN_AMBIENT_INTENSITY) * lerp;
}

float DayNightCycle::getNightFactor() const
{
    const float ambient = getAmbientIntensity();
    const float range = lighting::MAX_AMBIENT_INTENSITY - lighting::MIN_AMBIENT_INTENSITY;
    const float normalized = (ambient - lighting::MIN_AMBIENT_INTENSITY) / (range > 0.0f ? range : 1.0f);
    return 1.0f - std::clamp(normalized, 0.0f, 1.0f);
}

float DayNightCycle::getShadowFactor() const
{
    return std::clamp(getNightFactor(), 0.0f, 1.0f);
}

bool DayNightCycle::isNight() const
{
    return getNightFactor() >= 0.5f;
}


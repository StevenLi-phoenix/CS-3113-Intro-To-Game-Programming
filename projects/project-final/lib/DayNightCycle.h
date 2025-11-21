#ifndef DAY_NIGHT_CYCLE_H
#define DAY_NIGHT_CYCLE_H

#include "../constants.h"

class DayNightCycle
{
public:
    explicit DayNightCycle(float secondsPerDay = lighting::DEFAULT_DAY_LENGTH_SECONDS);

    void update(float deltaTime);

    void  setTimeScale(float scale);
    void  setNormalizedTime(float normalizedTime);
    void  setPaused(bool paused);
    float getSecondsPerDay() const { return mSecondsPerDay; }
    float getTimeScale() const { return mTimeScale; }
    bool  isPaused() const { return mPaused; }

    float getNormalizedTime() const;
    float getAmbientIntensity() const;
    float getNightFactor() const;
    float getShadowFactor() const;
    bool  isNight() const;

private:
    float mSecondsPerDay;
    float mCurrentTime;
    float mTimeScale;
    bool  mPaused;
};

#endif


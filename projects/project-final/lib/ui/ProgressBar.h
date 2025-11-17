#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "../Helper.h"
#include "uiBase.h"

// Usage example:
//     ProgressBar bar({screenWidth / 2.0f, 360.0f}, {300.0f, 24.0f});
//     bar.setRange(0.0f, 100.0f);
//     bar.setValue(50.0f);
//     bar.setBackgroundColor(Fade(GRAY, 0.1f));
//     bar.setFillColor(BLUE);
//     bar.setBorderColor(DARKBLUE);
//     // call bar.update(deltaTime) in your update loop
//     // call bar.render() inside BeginDrawing/EndDrawing

class ProgressBar : public UIBase
{
private:
    float mMinValue;
    float mMaxValue;
    float mValue;
    Color mBackgroundColor;
    Color mFillColor;
    Color mBorderColor;
    float mBorderThickness;

public:
    ProgressBar();
    ProgressBar(Vector2 position, Vector2 size);

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    void setRange(float minValue, float maxValue);
    void setValue(float value);
    float getValue() const { return mValue; }

    void setBackgroundColor(Color color) { mBackgroundColor = color; }
    void setFillColor(Color color) { mFillColor = color; }
    void setBorderColor(Color color) { mBorderColor = color; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
};

#endif

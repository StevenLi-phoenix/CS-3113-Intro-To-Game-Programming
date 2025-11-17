#ifndef SLIDER_H
#define SLIDER_H

#include "../Helper.h"
#include "../Entity.h"
#include <functional>
#include <vector>

class Slider : public Entity
{
public:
    using ValueCallback = std::function<void(float)>;

private:
    float mMinValue;
    float mMaxValue;
    float mValue;
    bool mIsDragging;

    Color mTrackColor;
    Color mFillColor;
    Color mKnobColor;
    Color mBorderColor;
    float mBorderThickness;

    ValueCallback mOnValueChanged;
    bool mSnapEnabled;
    float mSnapTolerance;
    std::vector<float> mSnapValues;

    static Slider* sCurrentHoveredSlider;

public:
    Slider();
    Slider(Vector2 position, Vector2 size, float minValue, float maxValue, float startValue);

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    bool isMouseOver() const;
    bool isDragging() const { return mIsDragging; }

    void setRange(float minValue, float maxValue);
    void setValue(float value, bool fireCallback = true);
    float getValue() const { return mValue; }

    void setTrackColor(Color color) { mTrackColor = color; }
    void setFillColor(Color color) { mFillColor = color; }
    void setKnobColor(Color color) { mKnobColor = color; }
    void setBorderColor(Color color) { mBorderColor = color; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
    void setOnValueChanged(ValueCallback callback) { mOnValueChanged = callback; }
    void setSnapEnabled(bool enabled) { mSnapEnabled = enabled; }
    void setSnapTolerance(float tolerance) { mSnapTolerance = tolerance < 0.0f ? 0.0f : tolerance; }
    void setSnapValues(const std::vector<float>& values);
    void addSnapValue(float value);
    void clearSnapValues() { mSnapValues.clear(); }

    static void updateGlobalCursor();

private:
    float getNormalizedValue() const;
    Rectangle getTrackRectangle() const;
    float applySnapping(float candidate) const;
};

#endif

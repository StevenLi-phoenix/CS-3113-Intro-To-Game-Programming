#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include "../Helper.h"
#include "uiBase.h"
#include <functional>
#include <string>

class ToggleButton : public UIBase
{
public:
    using ToggleCallback = std::function<void(bool)>;

private:
    std::string mOnText;
    std::string mOffText;
    Color mOnBackgroundColor;
    Color mOffBackgroundColor;
    Color mOnBorderColor;
    Color mOffBorderColor;
    Color mOnTextColor;
    Color mOffTextColor;
    int mFontSize;
    float mBorderThickness;
    bool mIsToggled;
    bool mIsPressed;
    float mPressAnimationTime;
    float mPressAnimationDuration;
    float mPressScaleFactor;
    ToggleCallback mOnToggle;

    static ToggleButton* sCurrentHoveredButton;

public:
    ToggleButton();
    ToggleButton(Vector2 position, Vector2 size, const std::string& onText, const std::string& offText = "");

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    void setOnText(const std::string& text) { mOnText = text; }
    void setOffText(const std::string& text) { mOffText = text; }
    void setOnBackgroundColor(Color color) { mOnBackgroundColor = color; }
    void setOffBackgroundColor(Color color) { mOffBackgroundColor = color; }
    void setOnBorderColor(Color color) { mOnBorderColor = color; }
    void setOffBorderColor(Color color) { mOffBorderColor = color; }
    void setOnTextColor(Color color) { mOnTextColor = color; }
    void setOffTextColor(Color color) { mOffTextColor = color; }
    void setFontSize(int size) { mFontSize = size; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
    void setOnToggle(ToggleCallback callback) { mOnToggle = callback; }

    bool isMouseOver() const;
    bool isClicked() const;
    bool isPressed() const { return mIsPressed; }
    bool isToggled() const { return mIsToggled; }
    void setToggled(bool toggled, bool fireCallback = true);

    static void updateGlobalCursor();
};

#endif

#ifndef BUTTON_H
#define BUTTON_H
#include "../Helper.h"
#include "../Entity.h"
#include <string>
#include <functional>

class Button : public Entity
{
public:
    using Callback = std::function<void()>;

private:
    std::string mText;
    Color mBackgroundColor;
    Color mBorderColor;
    Color mTextColor;
    int mFontSize;
    float mBorderThickness;
    Callback mOnClick;
    
    // Animation state
    bool mIsPressed;
    float mPressAnimationTime;
    float mPressAnimationDuration;
    float mPressScaleFactor;
    
    // Static cursor management
    static Button* sCurrentHoveredButton;

public:
    Button();
    Button(Vector2 position, Vector2 size, const std::string& text);
    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;
    
    void setText(const std::string& text) { mText = text; }
    void setBackgroundColor(Color color) { mBackgroundColor = color; }
    void setBorderColor(Color color) { mBorderColor = color; }
    void setTextColor(Color color) { mTextColor = color; }
    void setFontSize(int size) { mFontSize = size; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
    void setOnClick(Callback callback) { mOnClick = callback; }
    
    bool isMouseOver() const;
    bool isClicked() const;
    bool isPressed() const { return mIsPressed; }
    
    // Static method to update cursor for all buttons
    static void updateGlobalCursor();
};

#endif
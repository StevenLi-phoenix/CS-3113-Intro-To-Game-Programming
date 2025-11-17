#include "Button.h"

// Static member initialization
Button* Button::sCurrentHoveredButton = nullptr;

Button::Button() : mText("Button"), mBackgroundColor(LIGHTGRAY), mBorderColor(BLACK), 
                   mTextColor(BLACK), mFontSize(20), mBorderThickness(2.0f), 
                   mOnClick(nullptr), mIsPressed(false), mPressAnimationTime(0.0f),
                   mPressAnimationDuration(0.15f), mPressScaleFactor(0.95f)
{
    setIsActive(true);
    setCanCollide(false);
}

Button::Button(Vector2 position, Vector2 size, const std::string& text) 
    : mText(text), mBackgroundColor(LIGHTGRAY), mBorderColor(BLACK), 
      mTextColor(BLACK), mFontSize(20), mBorderThickness(2.0f),
      mOnClick(nullptr), mIsPressed(false), mPressAnimationTime(0.0f),
      mPressAnimationDuration(0.15f), mPressScaleFactor(0.95f)
{
    setPosition(position);
    setScale(size);
    setIsActive(true);
    setCanCollide(false);
}

void Button::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    if (!getIsActive()) return;
    
    // Update hover state for cursor management
    bool wasHovered = (sCurrentHoveredButton == this);
    bool isHovered = isMouseOver();
    
    if (isHovered && !wasHovered)
    {
        // This button is now being hovered
        sCurrentHoveredButton = this;
    }
    else if (!isHovered && wasHovered)
    {
        // This button is no longer hovered
        if (sCurrentHoveredButton == this)
        {
            sCurrentHoveredButton = nullptr;
        }
    }
    
    // Update press animation
    bool isCurrentlyPressed = isMouseOver() && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    
    if (isCurrentlyPressed && !mIsPressed)
    {
        // Just started pressing
        mIsPressed = true;
        mPressAnimationTime = 0.0f;
    }
    else if (!isCurrentlyPressed && mIsPressed)
    {
        // Just released
        mIsPressed = false;
        mPressAnimationTime = 0.0f;
    }
    
    // Update animation time
    if (mIsPressed)
    {
        mPressAnimationTime += deltaTime;
        if (mPressAnimationTime > mPressAnimationDuration)
        {
            mPressAnimationTime = mPressAnimationDuration;
        }
    }
    else
    {
        mPressAnimationTime -= deltaTime;
        if (mPressAnimationTime < 0.0f)
        {
            mPressAnimationTime = 0.0f;
        }
    }
    
    // Trigger callback when button is clicked
    if (isClicked() && mOnClick)
    {
        mOnClick();
    }
}

bool Button::isMouseOver() const
{
    if (!getIsActive()) return false;
    
    Vector2 position = getPosition();
    Vector2 scale = getScale();
    
    Rectangle buttonRect = {
        position.x - scale.x / 2.0f,
        position.y - scale.y / 2.0f,
        scale.x,
        scale.y
    };
    
    Vector2 mousePos = GetMousePosition();
    return PointInRectangle(mousePos, buttonRect);
}

bool Button::isClicked() const
{
    return isMouseOver() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Button::render()
{
    if (!getIsActive()) return;
    
    Vector2 position = getPosition();
    Vector2 originalScale = getScale();
    
    // Calculate animation scale (ease in/out)
    float animationProgress = mPressAnimationTime / mPressAnimationDuration;
    if (animationProgress > 1.0f) animationProgress = 1.0f;
    if (animationProgress < 0.0f) animationProgress = 0.0f;
    
    // Ease in-out cubic for smooth animation
    float easedProgress = animationProgress < 0.5f
        ? 4.0f * animationProgress * animationProgress * animationProgress
        : 1.0f - pow(-2.0f * animationProgress + 2.0f, 3.0f) / 2.0f;
    
    float currentScale = 1.0f - (1.0f - mPressScaleFactor) * easedProgress;
    Vector2 scale = {originalScale.x * currentScale, originalScale.y * currentScale};
    
    // Draw button background rectangle
    Rectangle buttonRect = {
        position.x - scale.x / 2.0f,
        position.y - scale.y / 2.0f,
        scale.x,
        scale.y
    };
    
    // Use slightly darker color when pressed
    Color bgColor = mBackgroundColor;
    if (mIsPressed)
    {
        bgColor = AdjustColorBrightness(mBackgroundColor, -0.2f);
    }
    else if (isMouseOver())
    {
        bgColor = AdjustColorBrightness(mBackgroundColor, 0.1f);
    }
    
    DrawFilledRectangle(buttonRect, bgColor);
    DrawRectangleBorder(buttonRect, mBorderThickness, mBorderColor);
    
    // Draw text centered in button
    if (!mText.empty())
    {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), mText.c_str(), mFontSize, 1.0f);
        Vector2 textPosition = {
            position.x - textSize.x / 2.0f,
            position.y - textSize.y / 2.0f
        };
        
        DrawText(mText.c_str(), textPosition.x, textPosition.y, mFontSize, mTextColor);
    }
}

void Button::updateGlobalCursor()
{
    if (sCurrentHoveredButton != nullptr && sCurrentHoveredButton->isMouseOver())
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        sCurrentHoveredButton = nullptr;
    }
}

void Button::shutdown()
{
    Entity::shutdown();
}
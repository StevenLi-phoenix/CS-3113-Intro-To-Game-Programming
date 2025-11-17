#include "ToggleButton.h"

ToggleButton* ToggleButton::sCurrentHoveredButton = nullptr;

ToggleButton::ToggleButton()
    : mOnText("ON"), mOffText("OFF"),
      mOnBackgroundColor(DARKGREEN), mOffBackgroundColor(LIGHTGRAY),
      mOnBorderColor(BLACK), mOffBorderColor(BLACK),
      mOnTextColor(RAYWHITE), mOffTextColor(BLACK),
      mFontSize(20), mBorderThickness(2.0f),
      mIsToggled(false), mIsPressed(false), mPressAnimationTime(0.0f),
      mPressAnimationDuration(0.15f), mPressScaleFactor(0.95f),
      mOnToggle(nullptr)
{
    setIsActive(true);
    setCanCollide(false);
}

ToggleButton::ToggleButton(Vector2 position, Vector2 size, const std::string& onText, const std::string& offText)
    : mOnText(onText), mOffText(offText.empty() ? onText : offText),
      mOnBackgroundColor(DARKGREEN), mOffBackgroundColor(LIGHTGRAY),
      mOnBorderColor(BLACK), mOffBorderColor(BLACK),
      mOnTextColor(RAYWHITE), mOffTextColor(BLACK),
      mFontSize(20), mBorderThickness(2.0f),
      mIsToggled(false), mIsPressed(false), mPressAnimationTime(0.0f),
      mPressAnimationDuration(0.15f), mPressScaleFactor(0.95f),
      mOnToggle(nullptr)
{
    setPosition(position);
    setScale(size);
    setIsActive(true);
    setCanCollide(false);
}

void ToggleButton::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    if (!getIsActive()) return;

    bool wasHovered = (sCurrentHoveredButton == this);
    bool hoverNow = isMouseOver();

    if (hoverNow && !wasHovered)
    {
        sCurrentHoveredButton = this;
    }
    else if (!hoverNow && wasHovered && sCurrentHoveredButton == this)
    {
        sCurrentHoveredButton = nullptr;
    }

    bool currentlyPressed = hoverNow && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    if (currentlyPressed && !mIsPressed)
    {
        mIsPressed = true;
        mPressAnimationTime = 0.0f;
    }
    else if (!currentlyPressed && mIsPressed)
    {
        mIsPressed = false;
        mPressAnimationTime = 0.0f;
    }

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

    if (isClicked())
    {
        Vector2 mousePos = GetMousePosition();
        bool activateRightHalf = mousePos.x >= getPosition().x;
        setToggled(activateRightHalf);
    }
}

bool ToggleButton::isMouseOver() const
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

bool ToggleButton::isClicked() const
{
    return isMouseOver() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void ToggleButton::setToggled(bool toggled, bool fireCallback)
{
    if (mIsToggled == toggled) return;
    mIsToggled = toggled;
    if (mOnToggle && fireCallback)
    {
        mOnToggle(mIsToggled);
    }
}

void ToggleButton::render()
{
    if (!getIsActive()) return;

    Vector2 position = getPosition();
    Vector2 originalScale = getScale();

    float animationProgress = mPressAnimationTime / mPressAnimationDuration;
    animationProgress = Clamp(animationProgress, 0.0f, 1.0f);

    float easedProgress = animationProgress < 0.5f
        ? 4.0f * animationProgress * animationProgress * animationProgress
        : 1.0f - pow(-2.0f * animationProgress + 2.0f, 3.0f) / 2.0f;

    float currentScale = 1.0f - (1.0f - mPressScaleFactor) * easedProgress;
    Vector2 scale = {originalScale.x * currentScale, originalScale.y * currentScale};

    Rectangle buttonRect = {
        position.x - scale.x / 2.0f,
        position.y - scale.y / 2.0f,
        scale.x,
        scale.y
    };

    // Base background and border
    DrawFilledRectangle(buttonRect, mOffBackgroundColor);
    DrawRectangleBorder(buttonRect, mBorderThickness, mOffBorderColor);

    // Highlighted half representing the active selection
    Rectangle highlightRect = buttonRect;
    highlightRect.width = buttonRect.width / 2.0f;
    if (mIsToggled)
    {
        highlightRect.x += highlightRect.width;
    }

    Color highlightColor = mOnBackgroundColor;
    if (mIsPressed)
    {
        highlightColor = AdjustColorBrightness(highlightColor, -0.2f);
    }
    else if (isMouseOver())
    {
        highlightColor = AdjustColorBrightness(highlightColor, 0.05f);
    }
    DrawFilledRectangle(highlightRect, highlightColor);
    DrawRectangleBorder(highlightRect, mBorderThickness, mOnBorderColor);

    // Draw option texts centered in each half
    const std::string leftText = mOffText.empty() ? mOnText : mOffText;
    const std::string rightText = mOnText;

    float halfWidth = buttonRect.width / 2.0f;
    float textHeightOffset = position.y;
    Vector2 leftTextSize = MeasureTextEx(GetFontDefault(), leftText.c_str(), mFontSize, 1.0f);
    Vector2 rightTextSize = MeasureTextEx(GetFontDefault(), rightText.c_str(), mFontSize, 1.0f);

    Vector2 leftPos = {
        buttonRect.x + halfWidth / 2.0f - leftTextSize.x / 2.0f,
        textHeightOffset - leftTextSize.y / 2.0f
    };

    Vector2 rightPos = {
        buttonRect.x + halfWidth + halfWidth / 2.0f - rightTextSize.x / 2.0f,
        textHeightOffset - rightTextSize.y / 2.0f
    };

    DrawText(leftText.c_str(), leftPos.x, leftPos.y, mFontSize, mIsToggled ? mOffTextColor : mOnTextColor);
    DrawText(rightText.c_str(), rightPos.x, rightPos.y, mFontSize, mIsToggled ? mOnTextColor : mOffTextColor);
}

void ToggleButton::updateGlobalCursor()
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

void ToggleButton::shutdown()
{
    Entity::shutdown();
}

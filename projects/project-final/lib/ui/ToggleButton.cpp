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
        setToggled(!mIsToggled);
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

    Color bgColor = mIsToggled ? mOnBackgroundColor : mOffBackgroundColor;
    Color borderColor = mIsToggled ? mOnBorderColor : mOffBorderColor;
    Color textColor = mIsToggled ? mOnTextColor : mOffTextColor;

    if (mIsPressed)
    {
        bgColor = AdjustColorBrightness(bgColor, -0.2f);
    }
    else if (isMouseOver())
    {
        bgColor = AdjustColorBrightness(bgColor, 0.1f);
    }

    DrawFilledRectangle(buttonRect, bgColor);
    DrawRectangleBorder(buttonRect, mBorderThickness, borderColor);

    const std::string& text = mIsToggled ? mOnText : (mOffText.empty() ? mOnText : mOffText);
    if (!text.empty())
    {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(), mFontSize, 1.0f);
        Vector2 textPosition = {
            position.x - textSize.x / 2.0f,
            position.y - textSize.y / 2.0f
        };

        DrawText(text.c_str(), textPosition.x, textPosition.y, mFontSize, textColor);
    }
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

#include "TextInput.h"
#include <algorithm>

TextInput::TextInput()
    : mText(""),
      mPlaceholder("Enter text..."),
      mIsFocused(false),
      mMaxLength(64),
      mFontSize(20),
      mBackgroundColor(Fade(RAYWHITE, 0.8f)),
      mBorderColor(DARKGRAY),
      mTextColor(BLACK),
      mPlaceholderColor(GRAY),
      mCursorColor(BLACK),
      mBorderThickness(2.0f),
      mCursorBlinkTimer(0.0f),
      mCursorBlinkInterval(0.5f),
      mOnSubmit(nullptr),
      mOnFocusChanged(nullptr),
      mBackspaceHoldTimer(0.0f),
      mBackspaceRepeatDelay(0.3f),
      mBackspaceRepeatInterval(0.05f)
{
    setIsActive(true);
    setCanCollide(false);
    setScale({240.0f, 36.0f});
}

TextInput::TextInput(Vector2 position, Vector2 size)
    : TextInput()
{
    setPosition(position);
    setScale(size);
}

void TextInput::setText(const std::string& text)
{
    if (text.size() > mMaxLength)
    {
        mText = text.substr(0, mMaxLength);
    }
    else
    {
        mText = text;
    }
}

bool TextInput::isMouseOver() const
{
    return UIBase::isMouseOver();
}

void TextInput::processInput()
{
    // Append typed characters
    for (int key = GetCharPressed(); key != 0; key = GetCharPressed())
    {
        if (key >= 32 && key <= 126 && mText.size() < mMaxLength)
        {
            mText.push_back(static_cast<char>(key));
        }
    }

    bool backspacePressed = IsKeyPressed(KEY_BACKSPACE);
    bool backspaceDown = IsKeyDown(KEY_BACKSPACE);

    if (backspacePressed && !mText.empty())
    {
        mText.pop_back();
        mBackspaceHoldTimer = 0.0f;
    }
    else if (backspaceDown && !mText.empty())
    {
        mBackspaceHoldTimer += getDeltaTime();
        if (mBackspaceHoldTimer >= mBackspaceRepeatDelay)
        {
            mText.pop_back();
            mBackspaceHoldTimer -= mBackspaceRepeatInterval;
            if (mBackspaceHoldTimer < mBackspaceRepeatDelay)
            {
                mBackspaceHoldTimer = mBackspaceRepeatDelay;
            }
        }
    }
    else
    {
        mBackspaceHoldTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        if (mOnSubmit)
        {
            mOnSubmit(mText);
        }
        setFocused(false);
    }
}

void TextInput::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive()) return;

    Rectangle bounds = getBounds();
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool mouseOver = PointInRectangle(GetMousePosition(), bounds);
    if (mousePressed)
    {
        setFocused(mouseOver);
    }

    if (mIsFocused)
    {
        processInput();
        mCursorBlinkTimer += deltaTime;
        if (mCursorBlinkTimer >= mCursorBlinkInterval * 2.0f)
        {
            mCursorBlinkTimer = 0.0f;
        }
    }
    else
    {
        mCursorBlinkTimer = 0.0f;
    }
}

void TextInput::render()
{
    if (!getIsActive()) return;

    Rectangle bounds = getBounds();
    DrawFilledRectangle(bounds, mBackgroundColor);
    DrawRectangleBorder(bounds, mBorderThickness, mBorderColor);

    const std::string& textToDraw = (!mText.empty() || mIsFocused) ? mText : mPlaceholder;
    Color textColor = (!mText.empty() || mIsFocused) ? mTextColor : mPlaceholderColor;

    Vector2 textSize = MeasureTextEx(GetFontDefault(), textToDraw.c_str(), mFontSize, 1.0f);
    Vector2 textPosition = {
        bounds.x + 8.0f,
        bounds.y + bounds.height / 2.0f - textSize.y / 2.0f
    };

    DrawTextEx(GetFontDefault(), textToDraw.c_str(), textPosition, mFontSize, 1.0f, textColor);

    if (mIsFocused)
    {
        bool showCursor = mCursorBlinkTimer <= mCursorBlinkInterval;
        if (showCursor)
        {
            Vector2 typedSize = MeasureTextEx(GetFontDefault(), mText.c_str(), mFontSize, 1.0f);
            float cursorX = textPosition.x + typedSize.x + 2.0f;
            Rectangle cursorRect = {
                cursorX,
                bounds.y + bounds.height / 2.0f - typedSize.y / 2.0f,
                2.0f,
                typedSize.y
            };
            DrawFilledRectangle(cursorRect, mCursorColor);
        }
    }
}

void TextInput::shutdown()
{
    Entity::shutdown();
}

void TextInput::setFocused(bool focused)
{
    if (mIsFocused == focused) return;
    mIsFocused = focused;
    mCursorBlinkTimer = 0.0f;
    if (!mIsFocused)
    {
        mBackspaceHoldTimer = 0.0f;
    }
    if (mOnFocusChanged)
    {
        mOnFocusChanged(mIsFocused);
    }
}

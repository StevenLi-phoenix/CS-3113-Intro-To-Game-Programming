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
      mOnSubmit(nullptr)
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

    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyDown(KEY_BACKSPACE)) && !mText.empty())
    {
        static float repeatDelay = 0.05f;
        static float repeatTimer = 0.0f;

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            mText.pop_back();
            repeatTimer = 0.0f;
        }
        else
        {
            repeatTimer += getDeltaTime();
            if (repeatTimer >= repeatDelay)
            {
                mText.pop_back();
                repeatTimer = 0.0f;
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        if (mOnSubmit)
        {
            mOnSubmit(mText);
        }
        mIsFocused = false;
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
        mIsFocused = mouseOver;
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

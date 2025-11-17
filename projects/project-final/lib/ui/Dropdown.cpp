#include "Dropdown.h"

Dropdown::Dropdown()
    : mOptions(),
      mSelectedIndex(-1),
      mHoveredIndex(-1),
      mIsOpen(false),
      mIsFocused(false),
      mItemHeight(32.0f),
      mFontSize(20),
      mBackgroundColor(Fade(RAYWHITE, 0.9f)),
      mBorderColor(DARKGRAY),
      mTextColor(BLACK),
      mHighlightColor(Fade(DARKBLUE, 0.2f)),
      mBorderThickness(2.0f),
      mOnSelectionChanged(nullptr)
{
    setIsActive(true);
    setCanCollide(false);
    setScale({220.0f, 36.0f});
}

Dropdown::Dropdown(Vector2 position, Vector2 size)
    : Dropdown()
{
    setPosition(position);
    setScale(size);
}

void Dropdown::setOptions(const std::vector<std::string>& options)
{
    mOptions = options;
    if (mSelectedIndex >= static_cast<int>(mOptions.size()))
    {
        mSelectedIndex = -1;
    }
}

void Dropdown::addOption(const std::string& option)
{
    mOptions.push_back(option);
}

void Dropdown::setSelectedIndex(int index, bool fireCallback)
{
    if (index < -1 || index >= static_cast<int>(mOptions.size()))
    {
        return;
    }
    if (mSelectedIndex == index)
    {
        return;
    }
    mSelectedIndex = index;
    if (mOnSelectionChanged && fireCallback && mSelectedIndex >= 0)
    {
        mOnSelectionChanged(mSelectedIndex, mOptions[mSelectedIndex]);
    }
}

Rectangle Dropdown::getMainRect() const
{
    return UIBase::getBounds();
}

Rectangle Dropdown::getOptionRect(int index) const
{
    Rectangle mainRect = getMainRect();
    return {
        mainRect.x,
        mainRect.y + mainRect.height + index * mItemHeight,
        mainRect.width,
        mItemHeight
    };
}

bool Dropdown::isMouseOverMain() const
{
    return UIBase::isMouseOver();
}

bool Dropdown::isMouseOverOption(int index) const
{
    if (!getIsActive()) return false;
    return PointInRectangle(GetMousePosition(), getOptionRect(index));
}

void Dropdown::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive()) return;

    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool mouseOverMain = isMouseOverMain();

    if (mousePressed)
    {
        if (mouseOverMain)
        {
            mIsOpen = !mIsOpen;
            mIsFocused = true;
        }
        else if (mIsOpen)
        {
            bool clickedOption = false;
            for (int i = 0; i < static_cast<int>(mOptions.size()); ++i)
            {
                if (isMouseOverOption(i))
                {
                    setSelectedIndex(i);
                    clickedOption = true;
                    break;
                }
            }
            mIsOpen = false;
            if (!clickedOption)
            {
                mIsFocused = false;
            }
        }
        else
        {
            mIsFocused = false;
        }
    }

    if (mIsOpen)
    {
        mHoveredIndex = -1;
        for (int i = 0; i < static_cast<int>(mOptions.size()); ++i)
        {
            if (isMouseOverOption(i))
            {
                mHoveredIndex = i;
                break;
            }
        }
    }
    else
    {
        mHoveredIndex = -1;
    }
}

void Dropdown::render()
{
    if (!getIsActive()) return;

    Rectangle mainRect = getMainRect();
    DrawFilledRectangle(mainRect, mBackgroundColor);
    DrawRectangleBorder(mainRect, mBorderThickness, mBorderColor);

    const char* displayText = (mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mOptions.size()))
        ? mOptions[mSelectedIndex].c_str()
        : "Select...";

    Vector2 textSize = MeasureTextEx(GetFontDefault(), displayText, mFontSize, 1.0f);
    Vector2 textPos = {
        mainRect.x + 8.0f,
        mainRect.y + mainRect.height / 2.0f - textSize.y / 2.0f
    };
    DrawText(displayText, textPos.x, textPos.y, mFontSize, mTextColor);

    // Draw dropdown arrow
    float arrowX = mainRect.x + mainRect.width - 16.0f;
    float arrowY = mainRect.y + mainRect.height / 2.0f;
    DrawTriangle(
        {arrowX - 6.0f, arrowY - 4.0f},
        {arrowX + 6.0f, arrowY - 4.0f},
        {arrowX, arrowY + 4.0f},
        mTextColor
    );

    if (mIsOpen)
    {
        Rectangle optionRect = getOptionRect(0);
        Rectangle listRect = {
            optionRect.x,
            optionRect.y,
            mainRect.width,
            mItemHeight * static_cast<float>(mOptions.size())
        };
        DrawFilledRectangle(listRect, ApplyAlpha(mBackgroundColor, 0.98f));
        DrawRectangleBorder(listRect, mBorderThickness, mBorderColor);

        for (int i = 0; i < static_cast<int>(mOptions.size()); ++i)
        {
            Rectangle rect = getOptionRect(i);
            if (i == mHoveredIndex || i == mSelectedIndex)
            {
                DrawFilledRectangle(rect, mHighlightColor);
            }
            DrawRectangleBorder(rect, 1.0f, mBorderColor);

            Vector2 optionSize = MeasureTextEx(GetFontDefault(), mOptions[i].c_str(), mFontSize, 1.0f);
            Vector2 optionPos = {
                rect.x + 8.0f,
                rect.y + rect.height / 2.0f - optionSize.y / 2.0f
            };
            DrawText(mOptions[i].c_str(), optionPos.x, optionPos.y, mFontSize, mTextColor);
        }
    }
}

void Dropdown::shutdown()
{
    Entity::shutdown();
}

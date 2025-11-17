#include "ListBox.h"

ListBox::ListBox()
    : mItems(),
      mSelectedIndex(-1),
      mHoveredIndex(-1),
      mItemHeight(28.0f),
      mFontSize(18),
      mBackgroundColor(Fade(RAYWHITE, 0.95f)),
      mBorderColor(DARKGRAY),
      mTextColor(BLACK),
      mHighlightColor(Fade(SKYBLUE, 0.4f)),
      mBorderThickness(2.0f),
      mOnSelectionChanged(nullptr)
{
    setIsActive(true);
    setCanCollide(false);
    setScale({220.0f, 160.0f});
}

ListBox::ListBox(Vector2 position, Vector2 size)
    : ListBox()
{
    setPosition(position);
    setScale(size);
}

void ListBox::setItems(const std::vector<std::string>& items)
{
    mItems = items;
    if (mSelectedIndex >= static_cast<int>(mItems.size()))
    {
        mSelectedIndex = -1;
    }
}

void ListBox::addItem(const std::string& item)
{
    mItems.push_back(item);
}

void ListBox::setSelectedIndex(int index, bool fireCallback)
{
    if (index < -1 || index >= static_cast<int>(mItems.size()))
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
        mOnSelectionChanged(mSelectedIndex, mItems[mSelectedIndex]);
    }
}

Rectangle ListBox::getItemRect(int index) const
{
    Rectangle bounds = UIBase::getBounds();
    return {
        bounds.x,
        bounds.y + index * mItemHeight,
        bounds.width,
        mItemHeight
    };
}

bool ListBox::isMouseOverItem(int index) const
{
    if (!getIsActive()) return false;
    return PointInRectangle(GetMousePosition(), getItemRect(index));
}

void ListBox::update(float deltaTime, Entity *player, Map *map, const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive()) return;

    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    mHoveredIndex = -1;

    for (int i = 0; i < static_cast<int>(mItems.size()); ++i)
    {
        if (isMouseOverItem(i))
        {
            mHoveredIndex = i;
            if (mousePressed)
            {
                setSelectedIndex(i);
            }
            break;
        }
    }
}

void ListBox::render()
{
    if (!getIsActive()) return;

    Rectangle bounds = getBounds();
    DrawFilledRectangle(bounds, mBackgroundColor);
    DrawRectangleBorder(bounds, mBorderThickness, mBorderColor);

    for (int i = 0; i < static_cast<int>(mItems.size()); ++i)
    {
        Rectangle itemRect = getItemRect(i);
        if (i == mHoveredIndex || i == mSelectedIndex)
        {
            DrawFilledRectangle(itemRect, mHighlightColor);
        }
        DrawRectangleBorder(itemRect, 1.0f, ApplyAlpha(mBorderColor, 0.3f));

        Vector2 textSize = MeasureTextEx(GetFontDefault(), mItems[i].c_str(), mFontSize, 1.0f);
        Vector2 textPos = {
            itemRect.x + 6.0f,
            itemRect.y + itemRect.height / 2.0f - textSize.y / 2.0f
        };
        DrawText(mItems[i].c_str(), textPos.x, textPos.y, mFontSize, mTextColor);
    }
}

void ListBox::shutdown()
{
    Entity::shutdown();
}

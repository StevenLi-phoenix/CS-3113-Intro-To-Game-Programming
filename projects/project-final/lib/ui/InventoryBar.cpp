#include "InventoryBar.h"

#include <algorithm>
#include <string>
#include <cmath>

namespace
{
    Rectangle FitRectWithinBounds(const Rectangle &bounds, const Rectangle &source)
    {
        Rectangle fitted = bounds;
        const float sourceWidth = std::abs(source.width);
        const float sourceHeight = std::abs(source.height);
        if (sourceWidth <= 0.0f || sourceHeight <= 0.0f || bounds.width <= 0.0f || bounds.height <= 0.0f)
        {
            return fitted;
        }

        const float sourceAspect = sourceWidth / sourceHeight;
        const float boundsAspect = bounds.width / bounds.height;
        if (sourceAspect > boundsAspect)
        {
            fitted.height = bounds.width / sourceAspect;
            fitted.y = bounds.y + (bounds.height - fitted.height) * 0.5f;
        }
        else
        {
            fitted.width = bounds.height * sourceAspect;
            fitted.x = bounds.x + (bounds.width - fitted.width) * 0.5f;
        }
        return fitted;
    }
}

InventoryBar::InventoryBar(Inventory *inventory)
    : mInventory(inventory)
{
    setIsActive(true);
    setCanCollide(false);
}

void InventoryBar::update(float deltaTime,
                          Entity *player,
                          Map *map,
                          const std::vector<Entity*> &collidableEntities)
{
    (void)deltaTime;
    (void)player;
    (void)map;
    (void)collidableEntities;

    if (!getIsActive() || !mInventory)
    {
        return;
    }
    handleInput();
}

void InventoryBar::render()
{
    if (!getIsActive() || !mInventory)
    {
        return;
    }

    Rectangle barRect = computeBarRect();
    setScale({barRect.width, barRect.height});
    setPosition({barRect.x + barRect.width * 0.5f, barRect.y + barRect.height * 0.5f});

    DrawRectangleRounded(barRect, 0.2f, 8, mBackground);
    DrawRectangleLinesEx(barRect, 2.0f, mBorder);

    const size_t slotCount = std::max(static_cast<size_t>(1), mInventory->getSlotCount());
    Rectangle firstSlot = {
        barRect.x + mPadding,
        barRect.y + mPadding,
        mSlotSize,
        mSlotSize
    };

    for (size_t i = 0; i < slotCount; ++i)
    {
        Rectangle slotRect = firstSlot;
        slotRect.x += i * (mSlotSize + mSlotSpacing);
        const InventorySlot &slot = mInventory->getSlot(i);
        const bool selected = (i == mInventory->getSelectedIndex());
        drawSlot(slot, slotRect, selected);
    }

    const InventorySlot &selectedSlot = mInventory->getSlot(mInventory->getSelectedIndex());
    drawSelectedLabel(selectedSlot, barRect);
}

Rectangle InventoryBar::computeBarRect() const
{
    const size_t slotCount = std::max(static_cast<size_t>(1),
                                      mInventory ? mInventory->getSlotCount() : static_cast<size_t>(1));
    const float width = mPadding * 2.0f +
                        (slotCount * mSlotSize) +
                        (slotCount - 1) * mSlotSpacing;
    const float height = mPadding * 2.0f + mSlotSize;
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());

    Rectangle rect;
    rect.width = width;
    rect.height = height;
    rect.x = (screenWidth - width) * 0.5f;
    rect.y = screenHeight - height - mBarOffset;
    return rect;
}

void InventoryBar::handleInput()
{
    const size_t slotCount = mInventory ? mInventory->getSlotCount() : 0;
    if (slotCount == 0)
    {
        return;
    }

    const int keyOffset = KEY_ONE;
    const size_t keyCount = std::min(slotCount, static_cast<size_t>(9));

    for (size_t i = 0; i < keyCount; ++i)
    {
        const KeyboardKey key = static_cast<KeyboardKey>(keyOffset + static_cast<int>(i));
        if (IsKeyPressed(key))
        {
            mInventory->setSelectedIndex(i);
            break;
        }
    }

    const float wheel = GetMouseWheelMove();
    if (wheel > 0.1f)
    {
        mInventory->selectPrevious();
    }
    else if (wheel < -0.1f)
    {
        mInventory->selectNext();
    }
}

void InventoryBar::drawSelectedLabel(const InventorySlot &slot, const Rectangle &barRect) const
{
    if (slot.label.empty())
    {
        return;
    }

    const int fontSize = 20;
    const Vector2 textSize = MeasureTextEx(GetFontDefault(),
                                           slot.label.c_str(),
                                           static_cast<float>(fontSize),
                                           1.0f);
    const Vector2 textPos = {
        barRect.x + (barRect.width - textSize.x) * 0.5f,
        barRect.y - textSize.y - 8.0f
    };
    DrawText(slot.label.c_str(), static_cast<int>(textPos.x), static_cast<int>(textPos.y), fontSize, RAYWHITE);
}

void InventoryBar::drawSlot(const InventorySlot &slot, const Rectangle &slotRect, bool selected) const
{
    const float roundness = 0.25f;
    const float thickness = selected ? 3.0f : 1.5f;
    Color fillColor = mSlotFill;
    if (selected)
    {
        fillColor = Fade(mSlotFill, 0.7f);
    }

    DrawRectangleRounded(slotRect, roundness, 6, fillColor);
    DrawRectangleLinesEx(slotRect, thickness, selected ? mSelectedBorder : Fade(mBorder, 0.65f));

    Rectangle iconRect = slotRect;
    iconRect.x += 6.0f;
    iconRect.y += 6.0f;
    iconRect.width -= 12.0f;
    iconRect.height -= 12.0f;

    if (slot.hasIcon())
    {
        Rectangle fitted = FitRectWithinBounds(iconRect, slot.iconSource);
        DrawTexturePro(*slot.icon, slot.iconSource, fitted, {0.0f, 0.0f}, 0.0f, slot.iconTint);
    }
    else if (slot.hasItem())
    {
        DrawRectangleRounded(iconRect, 0.2f, 6, Fade(slot.iconTint, 0.8f));
    }

    if (slot.quantity > 0)
    {
        const std::string quantityText = TextFormat("%d", slot.quantity);
        const int fontSize = 18;
        const Vector2 textSize = MeasureTextEx(GetFontDefault(),
                                               quantityText.c_str(),
                                               static_cast<float>(fontSize),
                                               1.0f);
        const Vector2 textPos = {
            slotRect.x + slotRect.width - textSize.x - 6.0f,
            slotRect.y + slotRect.height - textSize.y - 2.0f
        };
        DrawText(quantityText.c_str(),
                 static_cast<int>(textPos.x),
                 static_cast<int>(textPos.y),
                 fontSize,
                 WHITE);
    }
}


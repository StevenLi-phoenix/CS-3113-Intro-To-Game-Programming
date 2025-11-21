#ifndef INVENTORY_BAR_H
#define INVENTORY_BAR_H

#include "../Inventory.h"
#include "uiBase.h"

class InventoryBar : public UIBase
{
public:
    explicit InventoryBar(Inventory *inventory = nullptr);

    void setInventory(Inventory *inventory) { mInventory = inventory; }

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;

private:
    Inventory *mInventory = nullptr; // non-owning
    float mSlotSize = 56.0f;
    float mSlotSpacing = 8.0f;
    float mPadding = 12.0f;
    float mBarOffset = 32.0f;
    Color mBackground = Fade(BLACK, 0.4f);
    Color mBorder = Fade(WHITE, 0.35f);
    Color mSlotFill = Fade(BLACK, 0.35f);
    Color mSelectedBorder = WHITE;

    void handleInput();
    Rectangle computeBarRect() const;
    void drawSelectedLabel(const InventorySlot &slot, const Rectangle &barRect) const;
    void drawSlot(const InventorySlot &slot, const Rectangle &slotRect, bool selected) const;
};

#endif


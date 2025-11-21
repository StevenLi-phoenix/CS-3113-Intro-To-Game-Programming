#ifndef INVENTORY_H
#define INVENTORY_H

#include "Helper.h"
#include <string>
#include <vector>

struct InventorySlot
{
    std::string id;
    std::string label;
    Texture2D *icon = nullptr; // Non-owning pointer managed by ResourceManager
    Rectangle iconSource = {0.0f, 0.0f, 0.0f, 0.0f};
    Color iconTint = WHITE;
    int quantity = 0;

    bool hasIcon() const
    {
        return icon && icon->id > 0 && iconSource.width != 0.0f && iconSource.height != 0.0f;
    }

    bool hasItem() const
    {
        return hasIcon() || !id.empty() || quantity > 0;
    }
};

class Inventory
{
public:
    static constexpr size_t DEFAULT_SLOT_COUNT = 9;

    explicit Inventory(size_t slotCount = DEFAULT_SLOT_COUNT);

    size_t getSlotCount() const { return mSlots.size(); }
    size_t getSelectedIndex() const { return mSelectedIndex; }

    const InventorySlot &getSlot(size_t index) const;
    void setSlot(size_t index, const InventorySlot &slot);
    void clearSlot(size_t index);

    void setSelectedIndex(size_t index);
    void selectNext();
    void selectPrevious();

    bool stackItem(const std::string &id, int amount);
    bool placeItem(size_t index, const InventorySlot &slot);
    bool adjustQuantity(size_t index, int delta);

private:
    std::vector<InventorySlot> mSlots;
    size_t mSelectedIndex = 0;
};

#endif


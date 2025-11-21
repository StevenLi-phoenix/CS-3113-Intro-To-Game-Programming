#include "Inventory.h"

#include <algorithm>

Inventory::Inventory(size_t slotCount)
    : mSlots(slotCount > 0 ? slotCount : DEFAULT_SLOT_COUNT),
      mSelectedIndex(0)
{
}

const InventorySlot &Inventory::getSlot(size_t index) const
{
    static InventorySlot EMPTY_SLOT{};
    if (index >= mSlots.size())
    {
        return EMPTY_SLOT;
    }
    return mSlots[index];
}

void Inventory::setSlot(size_t index, const InventorySlot &slot)
{
    if (index >= mSlots.size())
    {
        return;
    }
    mSlots[index] = slot;
}

void Inventory::clearSlot(size_t index)
{
    if (index >= mSlots.size())
    {
        return;
    }
    mSlots[index] = InventorySlot{};
}

void Inventory::setSelectedIndex(size_t index)
{
    if (mSlots.empty())
    {
        mSelectedIndex = 0;
        return;
    }
    mSelectedIndex = std::min(index, mSlots.size() - 1);
}

void Inventory::selectNext()
{
    if (mSlots.empty())
    {
        return;
    }
    mSelectedIndex = (mSelectedIndex + 1) % mSlots.size();
}

void Inventory::selectPrevious()
{
    if (mSlots.empty())
    {
        return;
    }
    mSelectedIndex = (mSelectedIndex == 0)
        ? mSlots.size() - 1
        : mSelectedIndex - 1;
}

bool Inventory::stackItem(const std::string &id, int amount)
{
    if (id.empty() || amount <= 0)
    {
        return false;
    }

    for (auto &slot : mSlots)
    {
        if (slot.id == id && slot.quantity > 0)
        {
            slot.quantity += amount;
            return true;
        }
    }

    for (auto &slot : mSlots)
    {
        if (!slot.hasItem())
        {
            slot.id = id;
            slot.label = id;
            slot.quantity = amount;
            return true;
        }
    }

    return false;
}

bool Inventory::placeItem(size_t index, const InventorySlot &slot)
{
    if (index >= mSlots.size())
    {
        return false;
    }
    mSlots[index] = slot;
    return true;
}

bool Inventory::adjustQuantity(size_t index, int delta)
{
    if (index >= mSlots.size())
    {
        return false;
    }

    InventorySlot &slot = mSlots[index];
    const int updated = std::max(0, slot.quantity + delta);
    slot.quantity = updated;
    return true;
}


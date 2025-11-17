#ifndef LISTBOX_H
#define LISTBOX_H

#include "../Helper.h"
#include "uiBase.h"
#include <functional>
#include <string>
#include <vector>

// Usage example:
//     ListBox listBox({screenWidth / 4.0f, 360.0f}, {200.0f, 140.0f});
//     listBox.setItems({"Item A", "Item B", "Item C"});
//     listBox.setOnSelectionChanged([](int index, const std::string& value) {
//         LOG(TextFormat("ListBox selected %d: %s", index, value.c_str()));
//     });
//     // Call listBox.update(deltaTime) and listBox.render() each frame.

class ListBox : public UIBase
{
public:
    using SelectionCallback = std::function<void(int, const std::string&)>;

private:
    std::vector<std::string> mItems;
    int mSelectedIndex;
    int mHoveredIndex;
    float mItemHeight;
    int mFontSize;

    Color mBackgroundColor;
    Color mBorderColor;
    Color mTextColor;
    Color mHighlightColor;
    float mBorderThickness;

    SelectionCallback mOnSelectionChanged;

public:
    ListBox();
    ListBox(Vector2 position, Vector2 size);

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    void setItems(const std::vector<std::string>& items);
    void addItem(const std::string& item);
    const std::vector<std::string>& getItems() const { return mItems; }
    void setSelectedIndex(int index, bool fireCallback = true);
    int getSelectedIndex() const { return mSelectedIndex; }

    void setItemHeight(float height) { mItemHeight = height; }
    void setFontSize(int size) { mFontSize = size; }
    void setBackgroundColor(Color color) { mBackgroundColor = color; }
    void setBorderColor(Color color) { mBorderColor = color; }
    void setTextColor(Color color) { mTextColor = color; }
    void setHighlightColor(Color color) { mHighlightColor = color; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
    void setOnSelectionChanged(SelectionCallback callback) { mOnSelectionChanged = callback; }

private:
    Rectangle getItemRect(int index) const;
    bool isMouseOverItem(int index) const;
};

#endif

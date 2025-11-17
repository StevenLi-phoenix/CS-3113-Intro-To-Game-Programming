#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "../Helper.h"
#include "uiBase.h"
#include <functional>
#include <string>
#include <vector>

// Usage example:
//     Dropdown dropdown({screenWidth / 2.0f, 260.0f}, {220.0f, 36.0f});
//     dropdown.setOptions({"Easy", "Medium", "Hard"});
//     dropdown.setOnSelectionChanged([](int index, const std::string& value) {
//         LOG(TextFormat("Selected option %d: %s", index, value.c_str()));
//     });
//     // Call dropdown.update(deltaTime) and dropdown.render() inside your loop.

class Dropdown : public UIBase
{
public:
    using SelectionCallback = std::function<void(int, const std::string&)>;

private:
    std::vector<std::string> mOptions;
    int mSelectedIndex;
    int mHoveredIndex;
    bool mIsOpen;
    bool mIsFocused;
    float mItemHeight;
    int mFontSize;

    Color mBackgroundColor;
    Color mBorderColor;
    Color mTextColor;
    Color mHighlightColor;
    float mBorderThickness;

    SelectionCallback mOnSelectionChanged;

public:
    Dropdown();
    Dropdown(Vector2 position, Vector2 size);

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    void setOptions(const std::vector<std::string>& options);
    void addOption(const std::string& option);
    const std::vector<std::string>& getOptions() const { return mOptions; }
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
    Rectangle getMainRect() const;
    Rectangle getOptionRect(int index) const;
    bool isMouseOverMain() const;
    bool isMouseOverOption(int index) const;
};

#endif

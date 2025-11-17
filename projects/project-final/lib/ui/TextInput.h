#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "../Helper.h"
#include "uiBase.h"
#include <functional>
#include <string>

// Usage example:
//     TextInput input({screenWidth / 2.0f, 420.0f}, {280.0f, 36.0f});
//     input.setPlaceholder("Enter name...");
//     input.setOnSubmit([](const std::string& text) { LOG(TextFormat("Submitted: %s", text.c_str())); });
//     // In game loop: input.update(deltaTime); input.render();

class TextInput : public UIBase
{
public:
    using SubmitCallback = std::function<void(const std::string&)>;

private:
    std::string mText;
    std::string mPlaceholder;
    bool mIsFocused;
    size_t mMaxLength;
    int mFontSize;

    Color mBackgroundColor;
    Color mBorderColor;
    Color mTextColor;
    Color mPlaceholderColor;
    Color mCursorColor;
    float mBorderThickness;

    float mCursorBlinkTimer;
    float mCursorBlinkInterval;
    SubmitCallback mOnSubmit;

public:
    TextInput();
    TextInput(Vector2 position, Vector2 size);

    void update(float deltaTime, Entity *player = nullptr, Map *map = nullptr, const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;
    void shutdown() override;

    void setText(const std::string& text);
    const std::string& getText() const { return mText; }

    void setPlaceholder(const std::string& placeholder) { mPlaceholder = placeholder; }
    void setMaxLength(size_t maxLength) { mMaxLength = maxLength; }
    void setFontSize(int fontSize) { mFontSize = fontSize; }
    void setBackgroundColor(Color color) { mBackgroundColor = color; }
    void setBorderColor(Color color) { mBorderColor = color; }
    void setTextColor(Color color) { mTextColor = color; }
    void setPlaceholderColor(Color color) { mPlaceholderColor = color; }
    void setCursorColor(Color color) { mCursorColor = color; }
    void setBorderThickness(float thickness) { mBorderThickness = thickness; }
    void setOnSubmit(SubmitCallback callback) { mOnSubmit = callback; }

    bool isMouseOver() const;
    bool isFocused() const { return mIsFocused; }

private:
    void processInput();
};

#endif

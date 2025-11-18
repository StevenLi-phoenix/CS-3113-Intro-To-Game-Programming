// handle input from keyboard, mouse, gamepad, etc.
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Helper.h"
#include <functional>
#include <string>
#include <vector>

// Usage example:
//     Controller controller;
//     controller.bindAction("move_left", KEY_A, Controller::InputEvent::Held,
//         [](float) { player->moveLeft(); });
//     controller.bindAction("jump", KEY_SPACE, Controller::InputEvent::Pressed,
//         [](float) { player->jump(); });
//     controller.update(deltaTime); // call once per frame
//     // When text input is focused:
//     controller.setInputCaptureActive(true);  // pauses gameplay input
//     controller.setInputCaptureActive(false); // resume gameplay input

class Controller
{
public:
    enum class InputEvent
    {
        Pressed,
        Released,
        Held
    };

    using ActionCallback = std::function<void(float)>;

private:
    struct Binding
    {
        std::string actionName;
        KeyboardKey key;
        InputEvent eventType;
        ActionCallback callback;
    };

    std::vector<Binding> mBindings;
    bool mInputEnabled;
    bool mInputCaptured;

public:
    Controller();

    void bindAction(const std::string& actionName, KeyboardKey key, InputEvent eventType, ActionCallback callback);
    void unbindAction(const std::string& actionName);
    bool isActionBound(const std::string& actionName) const;

    void setInputEnabled(bool enabled);
    bool isInputEnabled() const { return mInputEnabled; }

    void setInputCaptureActive(bool active);
    bool isInputCaptured() const { return mInputCaptured; }

    void update(float deltaTime);
};

#endif

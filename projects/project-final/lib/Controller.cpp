#include "Controller.h"
#include <algorithm>

Controller::Controller()
    : mInputEnabled(true),
      mInputCaptured(false)
{
}

void Controller::bindAction(const std::string& actionName, KeyboardKey key, InputEvent eventType, ActionCallback callback)
{
    for (auto& binding : mBindings)
    {
        if (binding.actionName == actionName)
        {
            binding.key = key;
            binding.eventType = eventType;
            binding.callback = callback;
            return;
        }
    }

    mBindings.push_back({actionName, key, eventType, callback});
}

void Controller::unbindAction(const std::string& actionName)
{
    mBindings.erase(
        std::remove_if(
            mBindings.begin(),
            mBindings.end(),
            [&](const Binding& binding) { return binding.actionName == actionName; }),
        mBindings.end());
}

bool Controller::isActionBound(const std::string& actionName) const
{
    for (const auto& binding : mBindings)
    {
        if (binding.actionName == actionName)
        {
            return true;
        }
    }
    return false;
}

void Controller::setInputEnabled(bool enabled)
{
    mInputEnabled = enabled;
}

void Controller::setInputCaptureActive(bool active)
{
    mInputCaptured = active;
}

void Controller::update(float deltaTime)
{
    if (!mInputEnabled || mInputCaptured)
    {
        return;
    }

    for (const auto& binding : mBindings)
    {
        bool triggered = false;
        switch (binding.eventType)
        {
            case InputEvent::Pressed:
                triggered = IsKeyPressed(binding.key);
                break;
            case InputEvent::Released:
                triggered = IsKeyReleased(binding.key);
                break;
            case InputEvent::Held:
                triggered = IsKeyDown(binding.key);
                break;
        }

        if (triggered && binding.callback)
        {
            binding.callback(deltaTime);
        }
    }
}

// Settings overlay UI
#include "Settings.h"
#include "player.h"
#include "../lib/Music.h"
#include <algorithm>

Settings::Settings(Player* player, Controller* controller)
    : mPlayer(player),
      mController(controller),
      mVisible(false),
      mAwaitingKey(false),
      mPendingActionDef(nullptr),
      mActiveButton(nullptr),
      mPreviousExitKey(KEY_ESCAPE),
      mVolumeSlider(nullptr),
      mVolumeValue(100.0f),
      mVolumeLabel("Master Volume: 100%"),
      mGraphicsDropdown(nullptr),
      mGraphicsOptions({"Performance", "Balanced", "Quality"}),
      mGraphicsLabel("Graphics: Balanced")
{
    configureActions();
    mStatusMessage = "Click a control to rebind keys. Press F1 to close settings.";
}

Settings::~Settings()
{
    shutdown();
}

void Settings::configureActions()
{
    mActions.clear();
    mActions.push_back({
        "move_left",
        "Move Left",
        KEY_A,
        KEY_A,
        Controller::InputEvent::Held,
        [this](float) { if (mPlayer) mPlayer->moveLeft(); },
        nullptr
    });
    mActions.push_back({
        "move_right",
        "Move Right",
        KEY_D,
        KEY_D,
        Controller::InputEvent::Held,
        [this](float) { if (mPlayer) mPlayer->moveRight(); },
        nullptr
    });
    mActions.push_back({
        "move_up",
        "Move Up",
        KEY_W,
        KEY_W,
        Controller::InputEvent::Held,
        [this](float) { if (mPlayer) mPlayer->moveUp(); },
        nullptr
    });
    mActions.push_back({
        "move_down",
        "Move Down",
        KEY_S,
        KEY_S,
        Controller::InputEvent::Held,
        [this](float) { if (mPlayer) mPlayer->moveDown(); },
        nullptr
    });
}

void Settings::initialise()
{
    setupVolumeControl();
    setupGraphicsControl();
    buildUI();
    refreshButtonLabels();

    // apply default bindings
    for (auto& action : mActions)
    {
        applyBinding(action.name, action.defaultKey);
    }

    AudioManager::setMasterVolume(mVolumeValue / 100.0f);
}

void Settings::buildUI()
{
    float buttonWidth = 320.0f;
    float buttonHeight = 40.0f;
    float spacing = 12.0f;
    float totalHeight = static_cast<float>(mActions.size()) * (buttonHeight + spacing) - spacing;

    float startY = 320.0f;

    Vector2 start = {
        c::SCREEN_WIDTH / 2.0f,
        startY
    };

    for (size_t i = 0; i < mActions.size(); ++i)
    {
        float y = start.y + i * (buttonHeight + spacing);
        Button* button = new Button({start.x, y}, {buttonWidth, buttonHeight}, "");
        button->setOnClick([this, actionName = mActions[i].name]() {
            startRebind(actionName);
        });
        button->setBackgroundColor(Fade(DARKBLUE, 0.6f));
        button->setTextColor(RAYWHITE);
        button->setBorderColor(DARKBLUE);
        button->setZIndex(2);
        registerElement(button);
        mActions[i].button = button;
    }
}

void Settings::refreshButtonLabels()
{
    for (auto& action : mActions)
    {
        if (action.button)
        {
            setButtonHint(action, "");
        }
    }
}

void Settings::startRebind(const std::string& actionName)
{
    mAwaitingKey = true;
    mPendingAction = actionName;
    mPendingActionDef = findAction(actionName);
    mActiveButton = mPendingActionDef ? mPendingActionDef->button : nullptr;

    if (mPendingActionDef && mActiveButton)
    {
        setButtonHint(*mPendingActionDef, "Press a key...");
    }

    mPreviousExitKey = KEY_ESCAPE;
    SetExitKey(KEY_NULL);

    if (mController)
    {
        mController->setInputCaptureActive(true);
    }
}

bool Settings::captureKey()
{
    if (!mAwaitingKey) return false;

    int keyPressed = GetKeyPressed();
    if (keyPressed == 0)
    {
        return false;
    }

    if (keyPressed == KEY_ESCAPE)
    {
        cancelRebind();
        return true;
    }

    ActionDefinition* action = findAction(mPendingAction);
    if (!action)
    {
        cancelRebind();
        return true;
    }

    KeyboardKey newKey = static_cast<KeyboardKey>(keyPressed);

    if (isKeyConflict(newKey, action->name))
    {
        if (mActiveButton)
        {
            setButtonHint(*action, "Key already used!");
        }
        return true;
    }

    applyBinding(action->name, newKey);
    finishRebind();
    return true;
}

void Settings::applyBinding(const std::string& actionName, KeyboardKey key)
{
    for (auto& action : mActions)
    {
        if (action.name == actionName)
        {
            action.currentKey = key;
            if (mController)
            {
                mController->bindAction(action.name, key, action.eventType, action.callback);
            }
            refreshButtonLabels();
            break;
        }
    }
}

std::string Settings::keyToString(KeyboardKey key)
{
    if (key >= KEY_A && key <= KEY_Z)
    {
        char c = static_cast<char>('A' + (key - KEY_A));
        return std::string(1, c);
    }
    if (key >= KEY_ZERO && key <= KEY_NINE)
    {
        char c = static_cast<char>('0' + (key - KEY_ZERO));
        return std::string(1, c);
    }

    switch (key)
    {
        case KEY_LEFT: return "Left";
        case KEY_RIGHT: return "Right";
        case KEY_UP: return "Up";
        case KEY_DOWN: return "Down";
        case KEY_SPACE: return "Space";
        case KEY_ENTER: return "Enter";
        case KEY_TAB: return "Tab";
        case KEY_BACKSPACE: return "Backspace";
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT: return "Shift";
        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL: return "Ctrl";
        case KEY_LEFT_ALT:
        case KEY_RIGHT_ALT: return "Alt";
        default:
            return std::string(TextFormat("Key %d", key));
    }
}

void Settings::update(float deltaTime)
{
    if (!mVisible) return;

    if (mVolumeSlider)
    {
        mVolumeSlider->update(deltaTime);
    }
    if (mGraphicsDropdown)
    {
        mGraphicsDropdown->update(deltaTime);
    }

    for (auto& action : mActions)
    {
        if (action.button)
        {
            action.button->update(deltaTime);
        }
    }

    if (mAwaitingKey)
    {
        captureKey();
    }
}

void Settings::render()
{
    if (!mVisible) return;

    DrawRectangle(0, 0, c::SCREEN_WIDTH, c::SCREEN_HEIGHT, Fade(BLACK, 0.6f));

    DrawText("Settings", c::SCREEN_WIDTH / 2 - MeasureText("Settings", 36) / 2, 40, 36, RAYWHITE);
    DrawText(mStatusMessage.c_str(), c::SCREEN_WIDTH / 2 - MeasureText(mStatusMessage.c_str(), 20) / 2, 90, 20, RAYWHITE);

    if (mVolumeSlider)
    {
        DrawText(mVolumeLabel.c_str(),
                 c::SCREEN_WIDTH / 2 - MeasureText(mVolumeLabel.c_str(), 20) / 2,
                 140,
                 20,
                 RAYWHITE);
    }

    if (mGraphicsDropdown)
    {
        DrawText(mGraphicsLabel.c_str(),
                 c::SCREEN_WIDTH / 2 - MeasureText(mGraphicsLabel.c_str(), 20) / 2,
                 200,
                 20,
                 RAYWHITE);
    }

    std::vector<UIBase*> drawList;
    drawList.reserve(mUIElements.size());
    for (UIBase* element : mUIElements)
    {
        if (element && element->getIsActive())
        {
            drawList.push_back(element);
        }
    }

    std::sort(drawList.begin(), drawList.end(), [](UIBase* a, UIBase* b) {
        return a->getZIndex() < b->getZIndex();
    });

    for (UIBase* element : drawList)
    {
        element->render();
    }
}

void Settings::shutdown()
{
    delete mVolumeSlider;
    mVolumeSlider = nullptr;

    delete mGraphicsDropdown;
    mGraphicsDropdown = nullptr;

    for (auto& action : mActions)
    {
        if (action.button)
        {
            delete action.button;
            action.button = nullptr;
        }
    }
    mUIElements.clear();
}

void Settings::setupVolumeControl()
{
    mVolumeSlider = new Slider(
        {c::SCREEN_WIDTH / 2.0f, 170.0f},
        {320.0f, 20.0f},
        0.0f,
        100.0f,
        mVolumeValue
    );
    mVolumeSlider->setZIndex(1);
    registerElement(mVolumeSlider);
    mVolumeSlider->setSnapEnabled(true);
    mVolumeSlider->setSnapValues({0.0f, 25.0f, 50.0f, 75.0f, 100.0f});
    mVolumeSlider->setOnValueChanged([this](float value) {
        mVolumeValue = value;
        mVolumeLabel = TextFormat("Master Volume: %.0f%%", mVolumeValue);
        AudioManager::setMasterVolume(mVolumeValue / 100.0f);
    });
    mVolumeLabel = TextFormat("Master Volume: %.0f%%", mVolumeValue);
}

void Settings::setupGraphicsControl()
{
    mGraphicsDropdown = new Dropdown(
        {c::SCREEN_WIDTH / 2.0f, 240.0f},
        {260.0f, 36.0f}
    );
    mGraphicsDropdown->setZIndex(3);
    registerElement(mGraphicsDropdown);
    mGraphicsDropdown->setOptions(mGraphicsOptions);
    mGraphicsDropdown->setSelectedIndex(1, false);
    mGraphicsLabel = "Graphics: " + mGraphicsOptions[1] + " (placeholder)";
    mGraphicsDropdown->setOnSelectionChanged([this](int index, const std::string& value) {
        mGraphicsLabel = "Graphics: " + value + " (future feature)";
    });
}

void Settings::setVisible(bool visible)
{
    if (mVisible == visible) return;
    mVisible = visible;
    if (!mVisible)
    {
        mAwaitingKey = false;
        mPendingAction.clear();
        mStatusMessage = "Click a button to rebind keys. Press ESC to cancel.";
        cancelRebind();
    }
}

Settings::ActionDefinition* Settings::findAction(const std::string& actionName)
{
    for (auto& action : mActions)
    {
        if (action.name == actionName)
        {
            return &action;
        }
    }
    return nullptr;
}

bool Settings::isKeyConflict(KeyboardKey key, const std::string& actionName) const
{
    for (const auto& action : mActions)
    {
        if (action.name == actionName) continue;
        if (action.currentKey == key) return true;
    }
    return false;
}

void Settings::setButtonHint(Settings::ActionDefinition& action, const std::string& hintText)
{
    if (!action.button) return;
    std::string text;
    if (hintText.empty())
    {
        text = action.label + ": " + keyToString(action.currentKey);
    }
    else
    {
        text = action.label + ": " + hintText;
    }
    action.button->setText(text);
}

void Settings::cancelRebind()
{
    if (!mAwaitingKey) return;

    if (mPendingActionDef)
    {
        setButtonHint(*mPendingActionDef, "");
    }

    mAwaitingKey = false;
    mPendingAction.clear();
    mPendingActionDef = nullptr;
    mActiveButton = nullptr;

    SetExitKey(mPreviousExitKey);

    if (mController)
    {
        mController->setInputCaptureActive(mVisible);
    }
}

void Settings::finishRebind()
{
    if (mPendingActionDef)
    {
        setButtonHint(*mPendingActionDef, "");
    }
    mAwaitingKey = false;
    mPendingAction.clear();
    mPendingActionDef = nullptr;
    mActiveButton = nullptr;

    SetExitKey(mPreviousExitKey);

    if (mController)
    {
        mController->setInputCaptureActive(mVisible);
    }
}

void Settings::registerElement(UIBase* element)
{
    if (!element) return;
    mUIElements.push_back(element);
}

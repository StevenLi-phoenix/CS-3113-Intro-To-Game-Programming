// Settings overlay UI
#include "Settings.h"
#include "player.h"
#include "../lib/Music.h"
#include <algorithm>
#include <utility>

Settings::Settings(Player* player,
                   Controller* controller,
                   std::function<void()> retryCallback,
                   std::function<void(KeyboardKey)> retryKeyChanged,
                   std::function<void()> primaryActionCallback,
                   std::function<void(int)> difficultyChanged)
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
      mGraphicsLabel("Graphics: Balanced"),
      mHealthDropdown(nullptr),
      mHealthOptions({"Easy (100 HP)", "Normal (10 HP)", "Hard (3 HP)", "Impossible (1 HP)"}),
      mHealthValues({100.0f, 10.0f, 3.0f, 1.0f}),
      mHealthLabel("Difficulty: Normal (10 HP)"),
      mSelectedHealthIndex(1),
      mRetryCallback(retryCallback),
      mRetryKeyCallback(retryKeyChanged),
      mPrimaryActionCallback(primaryActionCallback),
      mDifficultyCallback(difficultyChanged)
{
    configureActions();
    mStatusMessage = "Click a control to rebind keys. Press F1 to close settings.";
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
    mActions.push_back({
        "primary_action",
        "Use Item",
        KEY_Z,
        KEY_Z,
        Controller::InputEvent::Pressed,
        [this](float) { if (mPrimaryActionCallback) mPrimaryActionCallback(); },
        nullptr
    });
    mActions.push_back({
        "retry_level",
        "Retry Level",
        KEY_ENTER,
        KEY_ENTER,
        Controller::InputEvent::Pressed,
        [this](float) { triggerRetry(); },
        nullptr
    });
}

void Settings::initialise()
{
    setupVolumeControl();
    setupGraphicsControl();
    setupHealthControl();
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
    const float leftColumnX = c::SCREEN_WIDTH * 0.35f;
    const float rightColumnX = c::SCREEN_WIDTH * 0.65f;
    const float leftColumnTop = 190.0f;
    const float leftColumnSpacing = 90.0f;

    if (mVolumeSlider)
    {
        mVolumeSlider->setPosition({leftColumnX, leftColumnTop});
    }
    if (mGraphicsDropdown)
    {
        mGraphicsDropdown->setPosition({leftColumnX, leftColumnTop + leftColumnSpacing});
    }
    if (mHealthDropdown)
    {
        mHealthDropdown->setPosition({leftColumnX, leftColumnTop + leftColumnSpacing * 2.0f});
    }

    const float buttonWidth = 320.0f;
    const float buttonHeight = 40.0f;
    const float spacing = 16.0f;
    const float bottomMargin = 40.0f;
    float startY = leftColumnTop;

    if (!mActions.empty())
    {
        const size_t lastIndex = mActions.size() - 1;
        float totalStep = static_cast<float>(lastIndex) * (buttonHeight + spacing);
        float lastButtonY = startY + totalStep;
        float lastButtonBottom = lastButtonY + buttonHeight * 0.5f;
        float bottomLimit = c::SCREEN_HEIGHT - bottomMargin;
        if (lastButtonBottom > bottomLimit)
        {
            float overflow = lastButtonBottom - bottomLimit;
            startY -= overflow;
        }
    }

    Vector2 start = {
        rightColumnX,
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
            if (action.name == "retry_level" && mRetryKeyCallback)
            {
                mRetryKeyCallback(key);
            }
            refreshButtonLabels();
            break;
        }
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
    if (mHealthDropdown)
    {
        mHealthDropdown->update(deltaTime);
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

    auto drawColumnLabel = [](const std::string& text, const Vector2& anchor, float offsetY) {
        if (text.empty()) return;
        int width = MeasureText(text.c_str(), 20);
        DrawText(text.c_str(),
                 static_cast<int>(anchor.x - width / 2),
                 static_cast<int>(anchor.y + offsetY),
                 20,
                 RAYWHITE);
    };

    if (mVolumeSlider)
    {
        drawColumnLabel(mVolumeLabel, mVolumeSlider->getPosition(), -38.0f);
    }

    if (mGraphicsDropdown)
    {
        drawColumnLabel(mGraphicsLabel, mGraphicsDropdown->getPosition(), -42.0f);
    }

    if (mHealthDropdown)
    {
        drawColumnLabel(mHealthLabel, mHealthDropdown->getPosition(), -42.0f);
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

    delete mHealthDropdown;
    mHealthDropdown = nullptr;

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
    mGraphicsDropdown->setZIndex(8);
    registerElement(mGraphicsDropdown);
    mGraphicsDropdown->setOptions(mGraphicsOptions);
    mGraphicsDropdown->setSelectedIndex(1, false);
    mGraphicsLabel = "Graphics: " + mGraphicsOptions[1] + " (placeholder)";
    mGraphicsDropdown->setOnSelectionChanged([this](int index, const std::string& value) {
        mGraphicsLabel = "Graphics: " + value + " (future feature)";
    });
}

void Settings::setupHealthControl()
{
    mHealthDropdown = new Dropdown(
        {c::SCREEN_WIDTH / 2.0f, 310.0f},
        {260.0f, 36.0f}
    );
    mHealthDropdown->setZIndex(3);
    registerElement(mHealthDropdown);
    mHealthDropdown->setOptions(mHealthOptions);
    mHealthDropdown->setSelectedIndex(mSelectedHealthIndex, false);
    mHealthDropdown->setOnSelectionChanged([this](int index, const std::string&) {
        mSelectedHealthIndex = index;
        applyHealthPreset(index);
    });
    applyHealthPreset(mSelectedHealthIndex);
}

void Settings::applyHealthPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(mHealthValues.size()))
    {
        return;
    }

    mHealthLabel = "Difficulty: " + mHealthOptions[index];
    if (mPlayer)
    {
        mPlayer->setMaxHealth(mHealthValues[index], true);
    }
    if (mDifficultyCallback)
    {
        mDifficultyCallback(index);
    }
}

void Settings::triggerRetry()
{
    if (mRetryCallback)
    {
        mRetryCallback();
    }
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

void Settings::updateContext(Player* player,
                             std::function<void()> retryCallback,
                             std::function<void(KeyboardKey)> retryKeyChanged,
                             std::function<void()> primaryActionCallback,
                             std::function<void(int)> difficultyChanged)
{
    mPlayer = player;
    mRetryCallback = std::move(retryCallback);
    mRetryKeyCallback = std::move(retryKeyChanged);
    mPrimaryActionCallback = std::move(primaryActionCallback);
    mDifficultyCallback = std::move(difficultyChanged);
    applyHealthPreset(mSelectedHealthIndex);
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
        text = action.label + ": " + KeyToString(action.currentKey);
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

#include "../lib/ui/ui.h"
#include "../lib/Helper.h"
#include "../lib/Scene.h"
#include "../lib/Controller.h"
#include "../constants.h"
#include <vector>
#include <string>
#include <functional>

#ifndef SETTINGS_H
#define SETTINGS_H

class Player;

class Settings : public Scene
{
public:
    Settings(Player* player,
             Controller* controller,
             std::function<void()> retryCallback = {},
             std::function<void(KeyboardKey)> retryKeyChanged = {});
    ~Settings() override = default;

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;

    void setVisible(bool visible);
    bool isVisible() const { return mVisible; }

private:
    struct ActionDefinition
    {
        std::string name;
        std::string label;
        KeyboardKey defaultKey;
        KeyboardKey currentKey;
        Controller::InputEvent eventType;
        Controller::ActionCallback callback;
        Button* button;
    };

    Player* mPlayer;
    Controller* mController;
    bool mVisible;
    bool mAwaitingKey;
    std::string mPendingAction;
    std::string mStatusMessage;
    std::vector<ActionDefinition> mActions;
    ActionDefinition* mPendingActionDef;
    Button* mActiveButton;
    int mPreviousExitKey;

    Slider* mVolumeSlider;
    float mVolumeValue;
    std::string mVolumeLabel;

    Dropdown* mGraphicsDropdown;
    std::vector<std::string> mGraphicsOptions;
    std::string mGraphicsLabel;

    Dropdown* mHealthDropdown;
    std::vector<std::string> mHealthOptions;
    std::vector<float> mHealthValues;
    std::string mHealthLabel;
    int mSelectedHealthIndex;

    std::function<void()> mRetryCallback;
    std::function<void(KeyboardKey)> mRetryKeyCallback;

    std::vector<UIBase*> mUIElements;

    void configureActions();
    void buildUI();
    void refreshButtonLabels();
    void startRebind(const std::string& actionName);
    bool captureKey();
    void applyBinding(const std::string& actionName, KeyboardKey key);
    static std::string keyToString(KeyboardKey key);

    void setupVolumeControl();
    void setupGraphicsControl();
    void setupHealthControl();
    void applyHealthPreset(int index);
    void triggerRetry();
    ActionDefinition* findAction(const std::string& actionName);
    bool isKeyConflict(KeyboardKey key, const std::string& actionName) const;
    void setButtonHint(ActionDefinition& action, const std::string& hintText);
    void cancelRebind();
    void finishRebind();
    void registerElement(UIBase* element);
};

#endif

#include "level_witchdemo.h"

#include "../lib/helper.h"

LevelWitchDemo::LevelWitchDemo()
{
}

LevelWitchDemo::~LevelWitchDemo()
{
    shutdown();
}

void LevelWitchDemo::initialise()
{
    mOrigin = {
        GetScreenWidth() / 2.0f,
        GetScreenHeight() / 2.0f
    };
    mBGColourHexCode = "#FFFFFF";
    mGameState.nextSceneID = 0;

    mWitch = new Witch();
    mGameState.xochitl = mWitch;

    if (mWitch)
    {
        mWitch->setGroundPlane(mWitch->getPosition().y);
    }
}

void LevelWitchDemo::handleInput()
{
    if (!mWitch) return;

    mWitch->beginInputFrame();

    if (IsKeyDown(KEY_A)) mWitch->moveLeft();
    if (IsKeyDown(KEY_D)) mWitch->moveRight();
    if (IsKeyPressed(KEY_SPACE)) mWitch->tryJump();

    if (!mWitch->controlsLocked())
    {
        if (IsKeyPressed(KEY_J) || IsKeyPressed(KEY_LEFT_CONTROL)) mWitch->playAttack();
        if (IsKeyPressed(KEY_F)) mWitch->playFall();
        if (IsKeyPressed(KEY_H)) mWitch->playHit();
        if (IsKeyPressed(KEY_K)) mWitch->playDeath();
        if (IsKeyPressed(KEY_T)) mWitch->playTurn();
        if (IsKeyPressed(KEY_R)) mWitch->playStartRun();
    }

    mWitch->finalizeInputFrame();
}

void LevelWitchDemo::update(float deltaTime)
{
    handleInput();

    if (mWitch)
    {
        mWitch->update(deltaTime, nullptr, nullptr, nullptr, 0);
    }
}

void LevelWitchDemo::renderOverlay() const
{
    DrawText("Controls:", 10, 10, 20, BLACK);
    DrawText("A/D - Run", 10, 35, 16, DARKGRAY);
    DrawText("SPACE - Jump", 10, 55, 16, DARKGRAY);
    DrawText("J / CTRL - Attack animation", 10, 75, 16, DARKGRAY);
    DrawText("F - Fall animation", 10, 95, 16, DARKGRAY);
    DrawText("H - Hit animation", 10, 115, 16, DARKGRAY);
    DrawText("K - Death animation", 10, 135, 16, DARKGRAY);
    DrawText("T - Turn animation", 10, 155, 16, DARKGRAY);
    DrawText("R - Start Run animation", 10, 175, 16, DARKGRAY);
}

void LevelWitchDemo::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    if (mWitch)
    {
        mWitch->render();
    }

    renderOverlay();
}

void LevelWitchDemo::shutdown()
{
    if (mWitch)
    {
        delete mWitch;
        mWitch = nullptr;
    }
    mGameState.xochitl = nullptr;
}

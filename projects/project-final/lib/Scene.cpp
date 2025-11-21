#include "Scene.h"
#include "../constants.h"

Scene::Scene() : mOrigin{{}}
{
    resetCamera();
}

Scene::Scene(Vector2 origin, const char *bgHexCode) : mOrigin{origin}, mBGColourHexCode {bgHexCode} 
{
    resetCamera();
    ClearBackground(ColorFromHex(bgHexCode));
}

void Scene::resetCamera()
{
    mGameState.camera.offset = { c::SCREEN_WIDTH / 2.0f, c::SCREEN_HEIGHT / 2.0f };
    mGameState.camera.target = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;
}

void Scene::setCameraFollowEnabled(bool enabled)
{
    mCameraFollowEnabled = enabled;
}

void Scene::updateCameraTarget(Vector2 target, float deltaTime)
{
    if (!mCameraFollowEnabled) return;

    float lerpAlpha = 1.0f - expf(-mCameraFollowSpeed * fmaxf(deltaTime, 0.0f));
    mGameState.camera.target = Vector2Lerp(mGameState.camera.target, target, lerpAlpha);
}

#include "Scene.h"
#include "../constants.h"
#include <algorithm>
#include <cmath>

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
    mCamera.offset = { c::SCREEN_WIDTH / 2.0f, c::SCREEN_HEIGHT / 2.0f };
    mCamera.target = mOrigin;
    mCamera.rotation = 0.0f;
    mCamera.zoom = 1.0f;
}

void Scene::setCameraFollowEnabled(bool enabled)
{
    mCameraFollowEnabled = enabled;
}

void Scene::updateCameraTarget(Vector2 target, float deltaTime)
{
    if (!mCameraFollowEnabled) return;

    float lerpAlpha = 1.0f - expf(-mCameraFollowSpeed * fmaxf(deltaTime, 0.0f));
    mCamera.target = Vector2Lerp(mCamera.target, target, lerpAlpha);
}

void Scene::setPaused(bool paused)
{
    mPaused = paused;
    mDayNightCycle.setPaused(paused);
}

void Scene::setChunkSize(int size)
{
    mChunkSize = std::max(1, size);
}

void Scene::setChunkLoadRadius(int radius)
{
    mChunkLoadRadius = std::max(0, radius);
}

bool Scene::updateStreamChunk(const Vector2 &worldPos, float tileSize, bool force)
{
    if (tileSize <= 0.0f) return false;

    const int tileX = static_cast<int>(std::floor(worldPos.x / tileSize));
    const int tileY = static_cast<int>(std::floor(worldPos.y / tileSize));
    const int chunkX = static_cast<int>(std::floor(static_cast<float>(tileX) / static_cast<float>(mChunkSize)));
    const int chunkY = static_cast<int>(std::floor(static_cast<float>(tileY) / static_cast<float>(mChunkSize)));

    if (!force && chunkX == mCurrentChunkX && chunkY == mCurrentChunkY)
    {
        return false;
    }

    mCurrentChunkX = chunkX;
    mCurrentChunkY = chunkY;
    const int radius = mChunkLoadRadius;
    mChunkStartX = (mCurrentChunkX - radius) * mChunkSize;
    mChunkStartY = (mCurrentChunkY - radius) * mChunkSize;
    return true;
}

void Scene::advanceDayNightCycle(float deltaTime)
{
    mDayNightCycle.update(deltaTime);
}

void Scene::initialiseLightingShader()
{
    if (mLightingReady)
    {
        return;
    }
    mLightingReady = mLightingShader.load("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
}

ShaderProgram* Scene::getLightingShader()
{
    if (!mLightingReady)
    {
        return nullptr;
    }
    return &mLightingShader;
}

float Scene::getTimeOfDay() const
{
    return mDayNightCycle.getNormalizedTime();
}

float Scene::getAmbientLight() const
{
    return mDayNightCycle.getAmbientIntensity();
}

float Scene::getNightFactor() const
{
    return mDayNightCycle.getNightFactor();
}

float Scene::getShadowFactor() const
{
    return mDayNightCycle.getShadowFactor();
}

bool Scene::isNightTime() const
{
    return mDayNightCycle.isNight();
}

void Scene::setLightingPaused(bool paused)
{
    mDayNightCycle.setPaused(paused);
}

#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

class Scene 
{
protected:
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    bool mCameraFollowEnabled = true;
    float mCameraFollowSpeed = 6.0f;
    Camera2D mCamera{};

    // chunk streaming helpers
    int mChunkSize = 128;
    int mChunkLoadRadius = 1; // one chunk in every direction
    int mCurrentChunkX = 0;
    int mCurrentChunkY = 0;
    int mChunkStartX = 0;
    int mChunkStartY = 0;
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    void resetCamera();
    void setCameraFollowEnabled(bool enabled);
    bool isCameraFollowEnabled() const { return mCameraFollowEnabled; }
    void updateCameraTarget(Vector2 target, float deltaTime);
    void setCameraFollowSpeed(float speed) { mCameraFollowSpeed = speed; }

    // chunk streaming
    void setChunkSize(int size);
    void setChunkLoadRadius(int radius);
    bool updateStreamChunk(const Vector2 &worldPos, float tileSize, bool force = false);
    int  getChunkStartX() const { return mChunkStartX; }
    int  getChunkStartY() const { return mChunkStartY; }
    int  getChunkSize()   const { return mChunkSize; }
    int  getChunkLoadRadius() const { return mChunkLoadRadius; }
    int  getChunkSpan() const { return mChunkLoadRadius * 2 + 1; }
    int  getLoadedColumns() const { return getChunkSpan() * mChunkSize; }
    int  getLoadedRows() const { return getChunkSpan() * mChunkSize; }

    const Camera2D& getCamera() const { return mCamera; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif

#ifndef COMPASS_H
#define COMPASS_H

#include "../lib/ui/uiBase.h"
#include "../lib/Helper.h"

class Compass : public UIBase
{
public:
    Compass();

    void setTarget(Entity *target) { mTarget = target; }
    void setPlayer(Entity *player) { mPlayer = player; }
    void setCamera(const Camera2D *camera) { mCamera = camera; }
    void setDistanceDisplay(float tileSize, float minDistanceTiles, float maxDistanceTiles);

    void update(float deltaTime,
                Entity *player = nullptr,
                Map *map = nullptr,
                const std::vector<Entity*> &collidableEntities = {}) override;
    void render() override;

private:
    Texture2D *mIcon = nullptr; // non-owning
    Rectangle mIconSource{0.0f, 0.0f, 0.0f, 0.0f};
    Entity *mTarget = nullptr; // non-owning
    Entity *mPlayer = nullptr; // non-owning
    const Camera2D *mCamera = nullptr; // non-owning
    float mArrowRadius = 140.0f;
    float mArrowThickness = 4.0f;
    Color mArrowColor = RAYWHITE;
    Color mBackground = Fade(BLACK, 0.35f);
    float mTileSize = 32.0f;
    float mMinDisplayTiles = 64.0f;
    float mMaxDisplayTiles = 256.0f;

    void loadIcon();
};

#endif // COMPASS_H

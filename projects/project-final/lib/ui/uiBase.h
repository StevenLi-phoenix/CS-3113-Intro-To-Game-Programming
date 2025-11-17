#ifndef UI_BASE_H
#define UI_BASE_H

#include "../Entity.h"

class UIBase {
public:
    UIBase();
    explicit UIBase(Entity *entity);
    virtual ~UIBase();

    void setEntity(Entity *entity);
    Entity* getEntity();
    const Entity* getEntity() const;

    virtual void update(float deltaTime);
    virtual void render();
    virtual void shutdown();

protected:
    Entity *mEntity = nullptr;
};

#endif

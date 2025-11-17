#include "uiBase.h"

UIBase::UIBase() = default;

UIBase::UIBase(Entity *entity) : mEntity(entity) {}

UIBase::~UIBase() = default;

void UIBase::setEntity(Entity *entity)
{
    mEntity = entity;
}

Entity* UIBase::getEntity()
{
    return mEntity;
}

const Entity* UIBase::getEntity() const
{
    return mEntity;
}

void UIBase::update(float deltaTime)
{
    if (mEntity) {
        mEntity->update(deltaTime);
    }
}

void UIBase::render()
{
    if (mEntity) {
        mEntity->render();
    }
}

void UIBase::shutdown()
{
    if (mEntity) {
        mEntity->shutdown();
    }
}

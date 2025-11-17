#include "uiBase.h"

UIBase::UIBase()
{
    setIsActive(true);
    setCanCollide(false);
}

UIBase::~UIBase() = default;

Rectangle UIBase::getBounds() const
{
    Vector2 position = getPosition();
    Vector2 size = getScale();
    return {
        position.x - size.x / 2.0f,
        position.y - size.y / 2.0f,
        size.x,
        size.y
    };
}

bool UIBase::containsPoint(Vector2 point) const
{
    return PointInRectangle(point, getBounds());
}

bool UIBase::isMouseOver() const
{
    if (!getIsActive()) return false;
    return containsPoint(GetMousePosition());
}

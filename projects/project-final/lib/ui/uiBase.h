#ifndef UI_BASE_H
#define UI_BASE_H

#include "../Entity.h"
#include "../Helper.h"

class UIBase : public Entity
{
public:
    UIBase();
    virtual ~UIBase();

    Rectangle getBounds() const;
    bool containsPoint(Vector2 point) const;
    bool isMouseOver() const;
};

#endif

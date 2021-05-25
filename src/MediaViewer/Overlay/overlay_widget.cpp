#include <QMouseEvent>
#include <QToolTip>

#include "overlay_widget.h"

Overlay_Widget::~Overlay_Widget(){};

bool Overlay_Widget::check_hover(QMouseEvent *e)
{
    if (rect.contains(e->pos()))
        return true;
    return false;
}


bool Overlay_Widget::activate_hover(QMouseEvent *e)
{
    if (!tooltip.isEmpty() && rect.contains(e->pos()))
        QToolTip::showText(e->globalPos(),tooltip);
    return false;
}


#include <QMouseEvent>
#include <QToolTip>

#include "overlay_widget.h"

bool Overlay_Widget::activate_hover(QMouseEvent *e)
{
    if (!tooltip.isEmpty() && rect.contains(e->pos()))
        QToolTip::showText(e->globalPos(),tooltip);
    return false;
}


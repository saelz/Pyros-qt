#include <QToolTip>
#include <QCursor>

#include "overlay_widget.h"

Overlay_Widget::~Overlay_Widget(){};

bool Overlay_Widget::check_hover(QPoint position)
{

    if (is_enabled && rect.contains(position))
        return true;
    return false;
}


bool Overlay_Widget::activate_hover(QPoint position)
{
    if (tooltip_enabled  && !tooltip.isEmpty() && rect.contains(position))
        QToolTip::showText(QCursor::pos(),tooltip);
    return false;
}


#ifndef OVERLAY_SPACER_H
#define OVERLAY_SPACER_H

#include "overlay_widget.h"

class Overlay_Spacer: public Overlay_Widget
{
public:
    Overlay_Spacer(int *available_space);
    inline int requested_width(QPainter &) override{return 0;};
    int draw(QPainter &p,int x, int y) override;
    int *unused_space;
};



#endif // OVERLAY_SPACER_H

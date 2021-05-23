#include "overlay_spacer.h"

Overlay_Spacer::Overlay_Spacer(int *available_space) : unused_space(available_space){}

int Overlay_Spacer::draw(QPainter &, int , int )
{
    if ((*unused_space) >= 0)
        return *unused_space;
    else
        return 0;
}

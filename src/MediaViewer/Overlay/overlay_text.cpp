#include "overlay.h"
#include "overlay_text.h"


Overlay_Text::Overlay_Text(QString tooltip,Overlay *parent): QObject(parent)
{
    this->tooltip = tooltip;
    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

void Overlay_Text::set_text(QString new_text)
{
    if (text != new_text){
        text = new_text;
        emit request_redraw();
    }
}

int Overlay_Text::requested_width(QPainter &p)
{
    return p.boundingRect(0,0,0,0,0,text).width()+padding;
}

int Overlay_Text::draw(QPainter &p,int x,int y)
{
    rect = p.boundingRect(x,y,0,0,0,text);
    rect.adjust(0,-rect.height(),0,-rect.height());
    p.drawText(rect,text);

    return rect.width()+padding;
}

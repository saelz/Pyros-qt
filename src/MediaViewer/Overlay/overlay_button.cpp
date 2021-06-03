#include <QToolTip>

#include "overlay.h"
#include "overlay_button.h"

Overlay_Button::Overlay_Button(QByteArray icon_path,bool *active_ptr,QString tooltip,Overlay *parent,bool toggleable,QByteArray icon_off_path) : QObject(parent),toggleable(toggleable),active(active_ptr)
{
    if (!icon_path.isEmpty()){
        icon = QImage(icon_path);
        icon = icon.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }

    if (toggleable && !icon_off_path.isEmpty()){
        icon_off = QImage(icon_off_path);
        icon_off = icon_off.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

        connect(this,&Overlay_Button::clicked,this,&Overlay_Button::toggle);
        connect(this,&Overlay_Button::toggle_changed,this,&Overlay_Button::request_redraw);
    } else if (toggleable && !icon.isNull()){

        icon_off = icon;

        for(int y = 0; y < icon_off.height(); y++)
            for(int x= 0; x < icon_off.width(); x++)
                icon_off.setPixelColor(x,y,QColor(155,155,155,icon_off.pixelColor(x,y).alpha()));

        connect(this,&Overlay_Button::clicked,this,&Overlay_Button::toggle);
        connect(this,&Overlay_Button::toggle_changed,this,&Overlay_Button::request_redraw);
    }

    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));

    this->tooltip = tooltip;
}

bool Overlay_Button::activate_hover(QPoint pos)
{
    bool inital_status = highlighed;
    if (rect.contains(pos)){
        highlighed = true;
        QToolTip::showText(QCursor::pos(),tooltip);
    } else {
        highlighed = false;
    }

   if (inital_status != highlighed){
       emit request_redraw();
   }

   return highlighed;
}

int Overlay_Button::requested_width(QPainter &)
{
    if (active == nullptr || (*active))
        return width+3;
    else
        return 0;
}

int Overlay_Button::draw(QPainter &p, int x, int y)
{
    if (active == nullptr || (*active)){
        rect = QRect(x,y-width,width,height);
        if (highlighed)
            p.drawRect(rect);

        if (!toggleable || is_toggled){
            p.drawImage(rect,icon.scaled(width,height));
        } else {
            p.drawImage(rect,icon_off.scaled(width,height));
        }


        return width+3;
    } else {
        rect = QRect(0,0,0,0);
        return 0;
    }
}


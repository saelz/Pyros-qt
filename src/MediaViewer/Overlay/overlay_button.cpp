#include <QToolTip>

#include "overlay.h"
#include "overlay_button.h"

Overlay_Button::Overlay_Button(QByteArray icon_path,bool *visible_ptr,QString tooltip,Overlay *parent,bool toggleable,QByteArray icon_off_path,QString text) :
    QObject(parent),toggleable(toggleable),text(text),visible(visible_ptr)
{
    if (!icon_path.isEmpty()){
        icon = QImage(icon_path);
        icon = icon.scaled(
                QSize(icon_width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }

    if (toggleable && !icon_off_path.isEmpty()){
        icon_off = QImage(icon_off_path);
        icon_off = icon_off.scaled(
                QSize(icon_width,height),
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
    if (rect.contains(pos) && is_enabled){
        highlighed = true;
        if (tooltip_enabled)
            QToolTip::showText(QCursor::pos(),tooltip);
    } else {
        highlighed = false;
    }

   if (inital_status != highlighed){
       emit request_redraw();
   }

   return highlighed;
}

int Overlay_Button::requested_width(QPainter &p)
{
    if (visible == nullptr || (*visible)){
        int button_width = right_padding;

        if (!text.isEmpty())
            button_width += p.boundingRect(0,0,0,0,0,text).width();
        if (!icon.isNull())
            button_width += icon_width;

        return button_width;
    } else {
        return 0;
    }
}

int Overlay_Button::draw(QPainter &p, int x, int y)
{
    if (visible == nullptr || (*visible)){
        rect = QRect(x,y-icon_width,requested_width(p)-right_padding,height);

        if (highlighed && is_enabled)
            p.drawRect(rect);

        if (!icon.isNull()){
            rect = QRect(x,y-icon_width,icon_width,height);
            if (!toggleable || is_toggled)
                p.drawImage(rect,icon.scaled(icon_width,height));
            else
                p.drawImage(rect,icon_off.scaled(icon_width,height));
        }

        if (!text.isEmpty()){
            if (!is_enabled)
                p.setPen(QColor(155,155,155));
            if (!icon.isNull())
                rect = p.boundingRect(x+icon_width,y,0,0,0,text);
            else
                rect = p.boundingRect(x,y,0,0,0,text);
            rect.adjust(0,-rect.height(),0,-rect.height());
            p.drawText(rect,text);
        }


        return requested_width(p);
    } else {
        rect = QRect(0,0,0,0);
        return 0;
    }
}


void Overlay_Button::set_enabled(bool enabled)
{
    is_enabled = enabled;
    if (!icon.isNull()){
        for(int y = 0; y < icon_off.height(); y++)
            for(int x= 0; x < icon_off.width(); x++)
                if (enabled)
                    icon.setPixelColor(x,y,QColor(255,255,255,icon_off.pixelColor(x,y).alpha()));
                else
                    icon.setPixelColor(x,y,QColor(155,155,155,icon_off.pixelColor(x,y).alpha()));
    }
    emit request_redraw();
}

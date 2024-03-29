#include "overlay_volume_button.h"

Overlay_Volume_Button::Overlay_Volume_Button(bool *visible_ptr,Overlay *parent): Overlay_Button(":/data/icons/volume_med",visible_ptr,"Volume",parent)
{
    icon_low = QImage(":/data/icons/volume_low");
    icon_mute = QImage(":/data/icons/mute.png");

    icon_low = icon_low.scaled(
                QSize(icon_width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    icon_mute = icon_mute.scaled(
                QSize(icon_width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    icon_med = icon;

    connect(this,&Overlay_Volume_Button::clicked,this,&Overlay_Volume_Button::volume_change_request);
    connect(this,&Overlay_Volume_Button::clicked,this,&Overlay_Volume_Button::toggle_popup);
    connect(this,&Overlay_Volume_Button::unselected,this,&Overlay_Volume_Button::hide_popup);
    connect(this,&Overlay_Volume_Button::middle_button_clicked,this,&Overlay_Volume_Button::toggle_mute);
}

bool Overlay_Volume_Button::check_hover(QPoint pos)
{
    if (Overlay_Widget::check_hover(pos))
        return true;

    return popup_visible && popup_rect.contains(pos);
}

int Overlay_Volume_Button::draw(QPainter &p,int x,int y)
{
    if (popup_visible){
        QBrush bg(QColor(80,80,80,255));
        popup_rect = QRect(x+rect.width()/4,y-rect.width()-3-100,rect.width()/2,100);
        QRect volume_rect = popup_rect.adjusted(0,100-volume_level,0,0);


        p.fillRect(popup_rect,QBrush(QColor(0,0,0,255)));
        p.fillRect(volume_rect,bg);
        p.drawRect(popup_rect);
    }

    if (has_audio){
        return Overlay_Button::draw(p,x,y);
    } else {
        if (visible == nullptr || (*visible)){
            rect = QRect(x,y-icon_width,icon_width,height);
            p.drawImage(rect,icon_mute.scaled(icon_width,height));
            return icon_width+3;
        } else {
            return 0;
        }
    }

}

bool Overlay_Volume_Button::activate_hover(QPoint pos)
{
    if (popup_visible && popup_rect.contains(pos)){
        hover_volume = (popup_rect.bottom()-pos.y())/(double)popup_rect.height();
        return true;
    } else {
        hover_volume = -1;
    }

    return Overlay_Button::activate_hover(pos);
}

bool Overlay_Volume_Button::scroll(QPoint angleDelta)
{
    if (!has_audio || angleDelta.y() == 0)
        return false;

    double new_vol = volume_level + angleDelta.y()/20;;

    if (new_vol > 100)
        new_vol = 100;
    else if (new_vol < 0)
        new_vol = 0;

    emit change_volume(new_vol/100);
    set_volume(new_vol);

    return true;
}

void Overlay_Volume_Button::set_volume(double vol)
{
    muted = false;
    volume_level = vol;

    if (vol > 50)
        icon = icon_med;
    else if (vol > 0)
        icon = icon_low;
    else
        icon = icon_mute;

    emit request_redraw();
}

void Overlay_Volume_Button::set_mute_state(bool is_muted)
{
    if (!has_audio)
        return;
    muted = is_muted;
    if (muted){
        icon = icon_mute;
        emit change_volume(0);
    } else {
        emit change_volume(volume_level/100);
        set_volume(volume_level);
    }

    emit request_redraw();
};

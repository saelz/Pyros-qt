#ifndef OVERLAY_VOLUME_BUTTON_H
#define OVERLAY_VOLUME_BUTTON_H

#include "overlay_button.h"

class Overlay_Volume_Button : public Overlay_Button{
    Q_OBJECT
    double volume_level = 100;
    double hover_volume = -1;
    bool popup_visible = false;
    bool has_audio = true;
    bool muted = false;
    QRect popup_rect;
public:
    Overlay_Volume_Button(bool *active_ptr,Overlay *parent);
    QImage icon_low;
    QImage icon_med;
    QImage icon_mute;
    bool activate_hover(QMouseEvent *e) override;

public slots:
    int draw(QPainter &p,int x, int y) override;
    inline void toggle_popup(){if (has_audio) popup_visible = !popup_visible;emit request_redraw();};
    void set_volume(double vol);
    void set_mute_state(bool is_muted);
    inline void toggle_mute(){set_mute_state(!muted);};
    inline void set_has_audio(bool audio){has_audio = audio;popup_visible = false;emit request_redraw();};
    bool check_hover(QMouseEvent *e) override;

private slots:
    inline void volume_change_request(){if (has_audio && hover_volume  != -1 && popup_visible){emit change_volume(hover_volume);set_volume(hover_volume*100);}};

signals:
    void clicked() override;
    void middle_button_clicked() override;
    void change_volume(double);
};

#endif // OVERLAY_VOLUME_BUTTON_H

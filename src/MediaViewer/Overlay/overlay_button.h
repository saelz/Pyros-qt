#ifndef OVERLAY_BUTTON_H
#define OVERLAY_BUTTON_H

#include "overlay_widget.h"

class Overlay;

class Overlay_Button :public QObject,public Overlay_Widget
{
    Q_OBJECT
public:
    Overlay_Button(QByteArray icon_path,bool *active_ptr,QString tooltip,Overlay *parent,bool toggleable = false,QByteArray off_icon = QByteArray());
    QImage icon;
    QImage icon_off;
    int width = 16;
    int height = 16;
    bool highlighed = false;
    bool toggleable;
    bool is_toggled = false;

    bool *active;

public slots:
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
    bool activate_hover(QMouseEvent *e) override;
    inline void set_toggle_state(bool state){is_toggled = state;emit request_redraw();};
    inline void toggle(){is_toggled = !is_toggled;emit toggle_changed();};

signals:
    void toggle_changed();
    void clicked() override;
    void request_redraw(void);

};

#endif // OVERLAY_BUTTON_H

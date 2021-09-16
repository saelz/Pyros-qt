#ifndef OVERLAY_BUTTON_H
#define OVERLAY_BUTTON_H

#include "overlay_widget.h"

class Overlay;

class Overlay_Button :public QObject,public Overlay_Widget
{
    Q_OBJECT

public:

    Overlay_Button(QByteArray icon_path,bool *visible_ptr,QString tooltip,Overlay *parent,
                   bool toggleable = false,QByteArray off_icon = QByteArray(),QString text=QString());
    QImage icon;
    QImage icon_off;
    int icon_width = 16;
    int height = 16;
    int right_padding = 5;
    bool highlighed = false;
    bool toggleable;
    bool is_toggled = false;
    QString text;

    bool *visible;

public slots:
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
    bool activate_hover(QPoint local_pos) override;

    inline void set_toggle_state(bool state){is_toggled = state;emit request_redraw();};
    inline void set_toggled(){set_toggle_state(true);};
    inline void set_not_toggled(){set_toggle_state(false);};
    inline void toggle(){is_toggled = !is_toggled;emit toggle_changed();};

    void set_enabled(bool enabled);

signals:
    void toggle_changed();
    void clicked() override;
    void middle_button_clicked() override;
    void right_button_clicked() override;
    void unselected() override;
    void request_redraw(void);

};

#endif // OVERLAY_BUTTON_H

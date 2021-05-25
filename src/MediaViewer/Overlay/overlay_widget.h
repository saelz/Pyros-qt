#ifndef OVERLAY_WIDGET_H
#define OVERLAY_WIDGET_H

#include <QRect>
#include <QPainter>
#include <QString>

class QMouseEvent;
class QPainter;

class Overlay_Widget
{
public:
    QRect rect = QRect(0,0,0,0);
    virtual ~Overlay_Widget();
    virtual int requested_width(QPainter &p) = 0;
    virtual int draw(QPainter &p,int x,int y) = 0;
    virtual bool activate_hover(QMouseEvent *e);
    virtual inline void clicked(){};
    virtual inline void unselected(){};
    virtual inline void middle_button_clicked(){clicked();};
    virtual bool check_hover(QMouseEvent *e);
    QString tooltip;
};
#endif // OVERLAY_WIDGET_H

#ifndef OVERLAY_TEXT_H
#define OVERLAY_TEXT_H


#include <QObject>
#include "overlay_widget.h"

class Overlay;

class Overlay_Text : public QObject,public Overlay_Widget
{
    Q_OBJECT
    QString text;
public:
    Overlay_Text(QString tooltip,Overlay *parent);
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
public slots:
    void set_text(QString text);

signals:
    void request_redraw(void);

};

#endif // OVERLAY_TEXT_H

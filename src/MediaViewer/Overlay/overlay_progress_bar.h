#ifndef OVERLAY_PROGRESS_BAR_H
#define OVERLAY_PROGRESS_BAR_H

#include "overlay_spacer.h"

class Overlay;

class Overlay_Progress_Bar :public QObject,public Overlay_Spacer
{
    Q_OBJECT
    double progress = 0;
    double hover_progress = 0;
public:
    Overlay_Progress_Bar(int *available_space,Overlay *parent);
    int draw(QPainter &p,int x, int y) override;
    void draw_progress(QPainter &p);
    bool activate_hover(QPoint local_pos) override;

public slots:
    void set_progress(int progress,int max);
private slots:
    inline void progress_change_request(){emit change_progress(hover_progress);};
signals:
    void clicked() override;
    void request_redraw(void);
    void change_progress(double);
};

#endif // OVERLAY_PROGRESS_BAR_H

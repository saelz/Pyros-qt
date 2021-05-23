#include <QMouseEvent>

#include "overlay.h"
#include "overlay_progress_bar.h"


Overlay_Progress_Bar::Overlay_Progress_Bar(int *available_space,Overlay *parent) : QObject(parent),Overlay_Spacer(available_space)
{
    connect(this,&Overlay_Progress_Bar::clicked,this,&Overlay_Progress_Bar::progress_change_request);
    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

bool Overlay_Progress_Bar::activate_hover(QMouseEvent *e)
{
    if (rect.contains(e->pos())){
        hover_progress = (e->pos().x()-rect.left())/(double)rect.width();
        return true;
    }

    return false;

}

int Overlay_Progress_Bar::draw(QPainter &p, int x, int y)
{
    if ((*unused_space) >= 10){
        rect = QRect(x,y-15,*unused_space-10,15);

        QRect progress_rect = rect;
        progress_rect.setWidth(progress/100*rect.width());

        QBrush highlight_bg(QColor(80,80,80,255));

        if (progress_rect.right() > progress_rect.left())
            p.fillRect(progress_rect,highlight_bg);

        p.drawRect(rect);

        return *unused_space;
    } else {
        return 0;
    }
}

void Overlay_Progress_Bar::set_progress(int new_progress,int max)
{
    progress = double(new_progress)/max*100;
    emit request_redraw();
}

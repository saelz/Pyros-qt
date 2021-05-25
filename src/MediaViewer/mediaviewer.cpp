#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QAction>
#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QFunctionPointer>

#include <pyros.h>

#include "mediaviewer.h"
#include "mpv_widget.h"
#include "../configtab.h"

#include "Overlay/overlay.h"

#include "unsupported_viewer.h"
#include "image_viewer.h"
#include "movie_viewer.h"
#include "text_viewer.h"
#include "cbz_viewer.h"
#include "video_viewer.h"

using ct = configtab;



MediaViewer::MediaViewer(QWidget *parent) : QWidget(parent)
{

    stacked_widget = new QStackedWidget();
    video_player = new mpv_widget;
    scroll_area = new QScrollArea;
    label = new QLabel();

    stacked_widget->insertWidget(VIDEO_LAYER,video_player);
    stacked_widget->insertWidget(LABEL_LAYER,scroll_area);

    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setWidgetResizable(true);

    setLayout(new QVBoxLayout);
    layout()->addWidget(stacked_widget);
    layout()->setContentsMargins(0,0,0,0);

    scroll_area->setLayout(new QVBoxLayout);
    scroll_area->layout()->setContentsMargins(0,0,0,0);
    scroll_area->setWidget(label);

    overlay = new Overlay(&viewer,this);
    overlay->setBaseSize(this->size());


    stacked_widget->setMouseTracking(true);
    scroll_area->setMouseTracking(true);
    label->setMouseTracking(true);
    video_player->setMouseTracking(true);

    setMouseTracking(true);

    QAction *pause = new QAction("Pause",this);
    QAction *seek_right = new QAction("Seek right",this);
    QAction *seek_left = new QAction("Seek left",this);
    QAction *toggle_mute = ct::create_binding(ct::KEY_TOGGLE_MUTE,"Toggle mute",this);

    pause->setShortcut(QKeySequence("Space"));
    seek_right->setShortcut(QKeySequence("Right"));
    seek_left->setShortcut(QKeySequence("Left"));

    pause->setAutoRepeat(false);

    addAction(pause);
    addAction(seek_right);
    addAction(seek_left);

    connect(pause,&QAction::triggered,overlay,&Overlay::pause);
    connect(seek_right,&QAction::triggered,overlay,&Overlay::fast_forward);
    connect(seek_left,&QAction::triggered,overlay,&Overlay::rewind);
    connect(toggle_mute,&QAction::triggered,overlay,&Overlay::toggle_playback_mute_state);

}

MediaViewer::~MediaViewer()
{
    if (viewer != nullptr)
        delete viewer;
}

void MediaViewer::set_file(PyrosFile* file)
{
   int used_layer = LABEL_LAYER;

     if (viewer != nullptr){
        delete viewer;
        viewer = nullptr;
    }

    if (file == nullptr)
        return;

    if (!strcmp(file->mime,"image/gif") &&
            !ct::setting_value(ct::GIFS_AS_VIDEO).toBool()){
        viewer = new Movie_Viewer(label);

    } else if (!qstrcmp(file->mime,"image/gif") ||
           !qstrncmp(file->mime,"audio/",6) ||
           !qstrncmp(file->mime,"video/",6)){
        used_layer = VIDEO_LAYER;
        viewer = new Video_Viewer(video_player);

    } else if (!qstrncmp(file->mime,"image/",6)){
        viewer = new Image_Viewer(label);

    } else if (!qstrcmp(file->mime,"application/zip") ||
           !qstrcmp(file->mime,"application/vnd.comicbook+zip")){
        viewer = new Cbz_Viewer(label);

    } else if (!qstrcmp(file->mime,"text/plain")){
        viewer = new Text_Viewer(label);

    } else {
        viewer = new Unsupported_Viewer(label);
    }

    if (viewer->always_show_vertical_scrollbar())
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    else
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    stacked_widget->setCurrentIndex(used_layer);

    viewer->set_file(file->path);
    update_scale();


    overlay->set_file(file);
    overlay->set_visible();

    // somtimes when the layer gets switched overlay won't be repainted, this forces a repaint
    QTimer::singleShot(10,overlay,SLOT(repaint()));

}

void MediaViewer::zoom_in()
{
    overlay->set_visible();
    if (viewer != nullptr){
        viewer->zoom_in();
        emit overlay->update_file_info(viewer->get_info());
    }
}

void MediaViewer::zoom_out()
{
    overlay->set_visible();
    if (viewer != nullptr){
        viewer->zoom_out();
        emit overlay->update_file_info(viewer->get_info());
    }
}

void MediaViewer::next_page()
{
    overlay->set_visible();
    if (viewer != nullptr && viewer->next_page()){
            scroll_area->verticalScrollBar()->setValue(0);
            emit overlay->update_file_info(viewer->get_info());
            overlay->repaint();
    }
}

void MediaViewer::prev_page()
{
    overlay->set_visible();
    if (viewer != nullptr && viewer->prev_page()){
            scroll_area->verticalScrollBar()->setValue(0);
            emit overlay->update_file_info(viewer->get_info());
    }
}

bool MediaViewer::is_dragable()
{

    if (viewer != nullptr && viewer->current_size().isValid() &&
            (viewer->current_size().width() > width() ||
             viewer->current_size().height() > height())){
        return true;
    }
    return false;
}

void MediaViewer::enterEvent(QEvent *e)
{
    if (is_dragable())
        setCursor(Qt::OpenHandCursor);

    overlay->auto_hide = false;
    overlay->set_visible();

    e->accept();
}

void MediaViewer::leaveEvent(QEvent *e)
{
    overlay->set_hidden();
    overlay->auto_hide = true;
    unsetCursor();

    e->accept();
}


void MediaViewer::update_scale()
{
    if (viewer != nullptr){
        viewer->resize(scroll_area->width()-1,scroll_area->height()-1,scale_type);
        emit overlay->update_file_info(viewer->get_info());
    }
}

void MediaViewer::set_scale(int scale)
{
    scale_type = (Viewer::SCALE_TYPE)scale;
    update_scale();
}

void MediaViewer::set_focus()
{
    if (stacked_widget->currentIndex() == VIDEO_LAYER)
        video_player->setFocus(Qt::OtherFocusReason);
    else
        scroll_area->setFocus(Qt::OtherFocusReason);
}

void MediaViewer::resizeEvent(QResizeEvent *e)
{
    if (scale_type != Viewer::SCALE_TYPE::ORIGINAL){
        update_scale();
    }

    overlay->setFixedSize(size());

    e->accept();

}

void MediaViewer::showEvent(QShowEvent *e)
{
    update_scale();
    e->accept();
}

void MediaViewer::mousePressEvent(QMouseEvent *e)
{
    if (overlay->mouseClicked(e)) return;

    if (is_dragable()){
        last_mouse_pos = e->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    set_focus();
    mouse_clicked = true;
    e->accept();
}

void MediaViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (overlay->mouseReleased(e)) return;

    if (is_dragable()){
        setCursor(Qt::OpenHandCursor);
    }
    mouse_clicked = false;
    e->accept();

}

void MediaViewer::mouseMoveEvent(QMouseEvent *e)
{
    if (!mouse_clicked ){
        if (overlay->mouseMoved(e)) {
            setCursor(Qt::PointingHandCursor);
            return;
        } else {
            unsetCursor();
        }
    }

    if (!mouse_clicked && is_dragable()){
        bool overlay_hovor = false;
        foreach(Overlay::Overlay_Bar *bar, overlay->overlay_bars)
            if (bar->active && bar->rect.contains(e->pos()))
                overlay_hovor = true;

        if (!overlay_hovor)
            setCursor(Qt::OpenHandCursor);
        else
            unsetCursor();
    }

    if (mouse_clicked && is_dragable()){
        QPoint diff = e->pos() - last_mouse_pos;

        QScrollBar  *hbar = scroll_area->horizontalScrollBar();
        QScrollBar  *vbar = scroll_area->verticalScrollBar();

        vbar->setValue(vbar->value()-diff.y());
        hbar->setValue(hbar->value()-diff.x());

        last_mouse_pos = e->pos();
        e->accept();
    }

}

void MediaViewer::bind_keys(QWidget *widget)
{
    QAction *lock_media_overlay = ct::create_binding(ct::KEY_LOCK_MEDIA_VIEWER_OVERLAY,"Lock Media Viewer Overlay",widget);
    QAction *zoom_in_bind = ct::create_binding(ct::KEY_ZOOM_IN,"Zoom in",widget);
    QAction *zoom_out_bind = ct::create_binding(ct::KEY_ZOOM_OUT,"Zoom out",widget);
    QAction *next_page_bind = ct::create_binding(ct::KEY_NEXT_PAGE,"Next page",widget);
    QAction *prev_page_bind = ct::create_binding(ct::KEY_PREV_PAGE,"Previous page",widget);
    QAction *focus_file_viewer = ct::create_binding(ct::KEY_FOCUS_FILE_VIEWER,"Focus file viewer",widget);

    connect(lock_media_overlay, &QAction::triggered,this, &MediaViewer::lock_overlay);
    connect(zoom_in_bind, &QAction::triggered,this, &MediaViewer::zoom_in);
    connect(zoom_out_bind, &QAction::triggered,this, &MediaViewer::zoom_out);
    connect(next_page_bind, &QAction::triggered,this, &MediaViewer::next_page);
    connect(prev_page_bind, &QAction::triggered,this, &MediaViewer::prev_page);
    connect(focus_file_viewer, &QAction::triggered,this, &MediaViewer::set_focus);
}

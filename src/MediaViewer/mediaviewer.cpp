#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QLocale>
#include <QTime>

#include <pyros.h>

#include "mediaviewer.h"
#include "mpv_widget.h"
#include "../configtab.h"

#include "unsupported_viewer.h"
#include "image_viewer.h"
#include "movie_viewer.h"
#include "text_viewer.h"
#include "cbz_viewer.h"
#include "video_viewer.h"

using ct = configtab;


Overlay::Overlay(Viewer **viewer,QWidget *parent) : QWidget(parent),viewer(viewer)
{
    auto_hide_timer.setSingleShot(true);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    raise();
    connect(&auto_hide_timer, &QTimer::timeout, this, &Overlay::set_hidden);
}

void Overlay::paintEvent(QPaintEvent *)
{
    if (state == HIDDEN)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QBrush bg(QColor(0,0,0,170));


    p.fillRect(QRect(0,rect().bottom()-20,rect().width(),20),bg);
    if (*viewer != nullptr){
        QDateTime timestamp;
        timestamp.setTime_t(file->import_time);

        QString info_text = (*viewer)->get_info();
        info_text += "  ";
        info_text += file->mime;
        info_text += "  ";
        info_text += locale().formattedDataSize(file->file_size);
        info_text += "  ";
        info_text += timestamp.toString(ct::setting_value(ct::TIMESTAMP).toString());

        p.drawText(0,rect().bottom()-5,info_text);
    }
}

void Overlay::set_visible()
{
    if (state != DISPLAYED){
        state = DISPLAYED;
        repaint();
    }

    if (auto_hide){
        if (auto_hide_timer.isActive())
            auto_hide_timer.stop();

        auto_hide_timer.start(1000);
    } else {
        auto_hide_timer.stop();
    }
}

void Overlay::set_hidden()
{
    if (state != HIDDEN){
        state = HIDDEN;
        repaint();
    }
}


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

}

MediaViewer::~MediaViewer()
{
    if (viewer != nullptr)
        delete viewer;
}

void MediaViewer::set_file(PyrosFile* file)
{
     if (viewer != nullptr){
        delete viewer;
        viewer = nullptr;
    }

    if (file == nullptr){
        return;
    }

    stacked_widget->setCurrentIndex(LABEL_LAYER);

    overlay->show();
    if (!strcmp(file->mime,"image/gif") &&
            !ct::setting_value(ct::GIFS_AS_VIDEO).toBool()){
        viewer = new Movie_Viewer(label);

    } else if (!qstrcmp(file->mime,"image/gif") ||
           !qstrncmp(file->mime,"audio/",6) ||
           !qstrncmp(file->mime,"video/",6)){
        stacked_widget->setCurrentIndex(VIDEO_LAYER);
        viewer = new Video_Viewer(video_player);
        overlay->hide();

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

    viewer->set_file(file->path);
    overlay->set_file(file);
    update_scale();

    if (viewer->always_show_vertical_scrollbar())
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    else
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    overlay->set_visible();

}

bool MediaViewer::is_resizable()
{
    if (viewer == nullptr)
        return false;

    return viewer->resizable();
}

bool MediaViewer::is_multipaged()
{
    if (viewer == nullptr)
        return false;

    return viewer->multi_paged();
}


void MediaViewer::zoom_in()
{
    overlay->set_visible();
    if (viewer != nullptr)
        viewer->zoom_in();
}

void MediaViewer::zoom_out()
{
    overlay->set_visible();
    if (viewer != nullptr)
        viewer->zoom_out();
}

void MediaViewer::next_page()
{
    overlay->set_visible();
    if (viewer != nullptr)
        if (viewer->next_page())
            scroll_area->verticalScrollBar()->setValue(0);
}

void MediaViewer::prev_page()
{
    overlay->set_visible();
    if (viewer != nullptr)
        if (viewer->prev_page())
            scroll_area->verticalScrollBar()->setValue(0);
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
    if (viewer != nullptr)
        viewer->resize(scroll_area->width()-1,scroll_area->height()-1,scale_type);
}

void MediaViewer::set_scale(SCALE_TYPE scale)
{
    scale_type = scale;
    update_scale();
}

void MediaViewer::set_focus()
{
    scroll_area->setFocus(Qt::OtherFocusReason);
}

void MediaViewer::resizeEvent(QResizeEvent *e)
{
    if (scale_type != SCALE_TYPE::ORIGINAL){
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
    if (is_dragable()){
        last_mouse_pos = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
    }
    set_focus();
}

void MediaViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (is_dragable()){
        setCursor(Qt::OpenHandCursor);
        e->accept();
    }

}

void MediaViewer::mouseMoveEvent(QMouseEvent *e)
{
    if (is_dragable()){
        QPoint diff = e->pos() - last_mouse_pos;

        QScrollBar  *hbar = scroll_area->horizontalScrollBar();
        QScrollBar  *vbar = scroll_area->verticalScrollBar();

        vbar->setValue(vbar->value()-diff.y());
        hbar->setValue(hbar->value()-diff.x());

        last_mouse_pos = e->pos();
        e->accept();
    }

}

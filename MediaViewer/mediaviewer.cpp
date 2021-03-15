#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QEvent>
#include <QVBoxLayout>

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


MediaViewer::MediaViewer(QWidget *parent) : QWidget(parent)
{

    stacked_widget = new QStackedWidget();
    video_player = new mpv_widget;
    scroll_area = new QScrollArea;
    label = new QLabel();

    stacked_widget->insertWidget(VIDEO_LAYER,video_player);
    stacked_widget->insertWidget(LABEL_LAYER,scroll_area);

    scroll_area->installEventFilter(this);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setWidgetResizable(true);

    setLayout(new QVBoxLayout);
    layout()->addWidget(stacked_widget);
    layout()->setContentsMargins(0,0,0,0);

    scroll_area->setLayout(new QVBoxLayout);
    scroll_area->layout()->setContentsMargins(0,0,0,0);
    scroll_area->setWidget(label);

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
        emit info_updated("");
        return;
    }

    stacked_widget->setCurrentIndex(LABEL_LAYER);

    if (!strcmp(file->mime,"image/gif") &&
            !ct::setting_value(ct::GIFS_AS_VIDEO).toBool()){
        viewer = new Movie_Viewer(label);

    } else if (!qstrcmp(file->mime,"image/gif") ||
           !qstrncmp(file->mime,"audio/",6) ||
           !qstrncmp(file->mime,"video/",6)){
        stacked_widget->setCurrentIndex(VIDEO_LAYER);
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

    viewer->set_file(file->path);
    emit info_updated(viewer->get_info());
    update_scale();
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    if (viewer != nullptr)
        viewer->zoom_in();
}

void MediaViewer::zoom_out()
{
    if (viewer != nullptr)
        viewer->zoom_out();
}

void MediaViewer::next_page()
{
    if (viewer != nullptr){
        if (viewer->next_page()){
            scroll_area->verticalScrollBar()->setValue(0);
            emit info_updated(viewer->get_info());
        }
    }
}

void MediaViewer::prev_page()
{
    if (viewer != nullptr){
        if (viewer->prev_page()){
            scroll_area->verticalScrollBar()->setValue(0);
            emit info_updated(viewer->get_info());
        }
    }
}

bool MediaViewer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        if (scale_type != SCALE_TYPE::ORIGINAL){
            update_scale();
        }
    }

    // enable scrollbars on hovor
    if (event->type() == QEvent::Enter) {
        switch (scale_type){
        case SCALE_TYPE::HEIGHT:
            scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        case SCALE_TYPE::WIDTH:
            scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        default:
            scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        }
    }

    if (event->type() == QEvent::Leave) {
        scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    if (viewer != nullptr && viewer->always_show_vertical_scrollbar())
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    return QWidget::eventFilter(obj, event);
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

void MediaViewer::set_focus(){
    scroll_area->setFocus(Qt::OtherFocusReason);
}

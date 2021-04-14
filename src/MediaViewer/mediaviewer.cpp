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
#include <QFunctionPointer>
#include <QToolTip>

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

Overlay_Button::Overlay_Button(QByteArray icon_path,bool *active_ptr,QString tooltip,Overlay *parent) : QObject(parent),active(active_ptr)
{
    if (!icon_path.isEmpty()){
        icon = QImage(icon_path);
        icon = icon.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }

    this->tooltip = tooltip;

    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

Overlay_Text::Overlay_Text(QString tooltip,Overlay *parent): QObject(parent)
{
    this->tooltip = tooltip;
    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

Overlay_Spacer::Overlay_Spacer(int *available_space) : unused_space(available_space){}

Overlay_Combo_Box::Overlay_Combo_Box(bool *active_ptr,QString tooltip,Overlay *parent) : Overlay_Button("",active_ptr,tooltip,parent)
{
    connect(this,&Overlay_Combo_Box::clicked,this,&Overlay_Combo_Box::toggle_drop_down);
}


void Overlay_Text::set_text(QString new_text)
{
    if (text != new_text){
        text = new_text;
        emit request_redraw();
    }
}

Overlay::Overlay(Viewer **viewer,MediaViewer *parent) : QWidget(parent),viewer(viewer)
{
    Overlay_Button *zoom_out_button = new Overlay_Button(":/data/icons/zoom_out.png",&show_zoom,"Zoom out",this);
    Overlay_Button *zoom_in_button = new Overlay_Button(":/data/icons/zoom_in.png",&show_zoom,"Zoom in",this);

    Overlay_Button *prev_page_button = new Overlay_Button(":/data/icons/prev.png",&show_page,"Previous page",this);
    Overlay_Button *next_page_button = new Overlay_Button(":/data/icons/next.png",&show_page,"Next page",this);

    Overlay_Text *info_text = new Overlay_Text("",this);
    Overlay_Text *mime_text = new Overlay_Text("Mime type",this);
    Overlay_Text *time_text = new Overlay_Text("Import time",this);
    Overlay_Text *size_text = new Overlay_Text("File size",this);

    Overlay_Spacer *spacer = new Overlay_Spacer(&unused_space);
    Overlay_Combo_Box *auto_scale = new Overlay_Combo_Box(&show_zoom,"Scale",this);

    auto_scale->entries.append({"Fit Both",Viewer::SCALE_TYPE::BOTH});
    auto_scale->entries.append({"Fit Height",Viewer::SCALE_TYPE::HEIGHT});
    auto_scale->entries.append({"Fit Width",Viewer::SCALE_TYPE::WIDTH});
    auto_scale->entries.append({"Original size",Viewer::SCALE_TYPE::ORIGINAL});

    connect(zoom_out_button,&Overlay_Button::clicked,parent,&MediaViewer::zoom_out);
    connect(zoom_in_button,&Overlay_Button::clicked,parent,&MediaViewer::zoom_in);

    connect(prev_page_button,&Overlay_Button::clicked,parent,&MediaViewer::prev_page);
    connect(next_page_button,&Overlay_Button::clicked,parent,&MediaViewer::next_page);

    connect(this,&Overlay::update_file_info,info_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_mime_info,mime_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_time_info,time_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_size_info,size_text,&Overlay_Text::set_text);

    connect(auto_scale,&Overlay_Combo_Box::entry_changed,parent,&MediaViewer::set_scale);


    overlay_widgets.append(zoom_out_button);
    overlay_widgets.append(zoom_in_button);
    overlay_widgets.append(prev_page_button);
    overlay_widgets.append(next_page_button);
    overlay_widgets.append(auto_scale);
    overlay_widgets.append(spacer);
    overlay_widgets.append(info_text);
    overlay_widgets.append(size_text);
    overlay_widgets.append(mime_text);
    overlay_widgets.append(time_text);

    auto_hide_timer.setSingleShot(true);
    connect(&auto_hide_timer, &QTimer::timeout, this, &Overlay::set_hidden);

    setAttribute(Qt::WA_TransparentForMouseEvents);

    raise();

}


void Overlay::paintEvent(QPaintEvent *)
{
    if (state == HIDDEN)
        return;

    QPainter p(this);
    QBrush bg(QColor(0,0,0,170));

    p.setRenderHints(QPainter::Antialiasing);

    p.setPen(Qt::white);

    p.fillRect(QRect(0,rect().bottom()-height,rect().width(),height),bg);
    if (*viewer != nullptr){
        int used_space = 0;
        int offset = left_margin;
        int y = rect().bottom()-bottom_margin;

        foreach(Overlay_Widget *widget,overlay_widgets){
            used_space += widget->requested_width(p);
        }

        qDebug("USED SPACE %d/%d DIFF: %d",used_space,width(),width()-used_space);
        unused_space = width()-used_space;

        foreach(Overlay_Widget *widget,overlay_widgets)
            offset += widget->draw(p,offset,y);


    }
}

void Overlay_Widget::check_hover(QMouseEvent *e)
{
    if (!tooltip.isEmpty() && rect.contains(e->pos()))
        QToolTip::showText(e->globalPos(),tooltip);
}

void Overlay_Button::check_hover(QMouseEvent *e)
{
    bool inital_status = highlighed;
    if (rect.contains(e->pos())){
        highlighed = true;
        QToolTip::showText(e->globalPos(),tooltip);
    } else {
        highlighed = false;
    }

   if (inital_status != highlighed){
       emit request_redraw();
   }
}

void Overlay_Combo_Box::check_hover(QMouseEvent *e)
{
    bool inital_status = highlighed;
    int last_highlighted_entry = highlighted_entry;


    if (rect.contains(e->pos())){
        highlighed = true;
        if (!display_dropdown)
            QToolTip::showText(e->globalPos(),tooltip);
    } else {
        highlighed = false;
    }


        highlighted_entry = -1;
    for(int i = 0;i < dropdownrect.length();i++){
        if (dropdownrect[i].contains(e->pos())){
            if (selected_entry  > i)
                highlighted_entry = i;
            else
                highlighted_entry = i+1;

            break;
        }
    }

    if (inital_status != highlighed ||
            last_highlighted_entry != highlighted_entry){
        emit request_redraw();
    }
}

int Overlay_Text::requested_width(QPainter &p)
{
    return p.boundingRect(0,0,0,0,0,text).width()+5;
}

int Overlay_Button::requested_width(QPainter &)
{
    if (active == nullptr || (*active))
        return width+3;
    else
        return 0;
}

int Overlay_Combo_Box::requested_width(QPainter &p)
{
    if (!entries.isEmpty())
        return p.boundingRect(0,0,0,0,0,entries[selected_entry].name).width()+3;
    else
        return 0;
}

int Overlay_Button::draw(QPainter &p, int x, int y)
{
    if (active == nullptr || (*active)){
        rect = QRect(x,y-width,width,height);
        if (highlighed)
            p.drawRect(rect);


        p.drawImage(rect,icon.scaled(width,height));

        return width+3;
    } else {
        rect = QRect(0,0,0,0);
        return 0;
    }
}

int Overlay_Text::draw(QPainter &p,int x,int y)
{
    rect = p.boundingRect(x,y,0,0,0,text);
    rect.adjust(0,-rect.height(),0,-rect.height());
    p.drawText(rect,text);

    return rect.width()+5;
}

int Overlay_Spacer::draw(QPainter &, int , int )
{
    if ((*unused_space) >= 0)
        return *unused_space;
    else
        return 0;
}

int Overlay_Combo_Box::draw(QPainter &p,int x,int y)
{
    if ((active == nullptr || (*active)) && !entries.isEmpty()){
        Combo_Entry current_entry = entries[selected_entry];
        int used_width;

        rect = p.boundingRect(x,y,0,0,0,current_entry.name);

        rect.adjust(0,-rect.height(),0,-rect.height());
        if (highlighed || highlighted_entry != -1)
            p.drawRect(rect);

        p.drawText(rect,current_entry.name);
        used_width = rect.width();

        if (display_dropdown){
            QBrush bg(QColor(0,0,0,255));
            QBrush highlight_bg(QColor(40,40,40,255));
            QRect bounding_text = rect;
            y -= bounding_text.height();

            int longest_drop_down = 0;
            for(int i = 1;i < entries.length();i++)
                if (entries[longest_drop_down].name.length() < entries[i].name.length())
                    longest_drop_down = i;

            int dropdown_width = p.boundingRect(0,0,0,0,0,entries[longest_drop_down].name).width();


            for(int i = 0;i < entries.length();i++){
                if (entries[i].value != current_entry.value){
                    y -= bounding_text.height();

                    bounding_text = p.boundingRect(bounding_text.x(),y,0,0,0,entries[i].name);
                    bounding_text.setWidth(dropdown_width);

                    if (highlighted_entry == i)
                        p.fillRect(bounding_text,highlight_bg);
                    else
                        p.fillRect(bounding_text,bg);

                    p.drawText(bounding_text,entries[i].name);
                    dropdownrect.append(bounding_text);
                }
            }

            rect.adjust(0,-bounding_text.height()*dropdownrect.length(),0,0);
            if (rect.width() > dropdown_width)
                rect.setWidth(dropdown_width);
        }

        return used_width+3;

    } else {
        return 0;
    }

}

void Overlay_Combo_Box::toggle_drop_down()
{
    if (display_dropdown){
        dropdownrect.clear();
        if (highlighted_entry != -1){
            selected_entry = highlighted_entry;
            emit entry_changed(entries[highlighted_entry].value);
        }

        highlighted_entry = -1;

    }

    display_dropdown = !display_dropdown;
    emit request_redraw();
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

        auto_hide_timer.start(3000);
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

void Overlay::set_file(PyrosFile *file)
{
    this->file = file;
    if (viewer != nullptr){
        QDateTime timestamp;
        timestamp.setTime_t(file->import_time);

        emit update_file_info((*viewer)->get_info());
        emit update_mime_info(file->mime);
        emit update_time_info(locale().formattedDataSize(file->file_size));
        emit update_size_info(timestamp.toString(ct::setting_value(ct::TIMESTAMP).toString()));

        show_zoom = (*viewer)->scaleing();
        show_page = (*viewer)->multi_paged();
    } else {
        show_page = false;
        show_zoom = false;
        emit update_file_info("");
        emit update_mime_info("");
        emit update_time_info("");
        emit update_size_info("");
    }
}

bool Overlay::mouseMoved(QMouseEvent *e)
{
    foreach(Overlay_Widget *widget,overlay_widgets)
        widget->check_hover(e);

    return false;
}

bool Overlay::mouseClicked(QMouseEvent *e)
{
    bool result = false;

    foreach(Overlay_Widget *widget,overlay_widgets){
        if (widget->rect.contains(e->pos())){
            last_pressed_widget = widget;
            result = true;
        }
    }

    return result;
}

bool Overlay::mouseReleased(QMouseEvent *e)
{
    bool result = false;
    foreach(Overlay_Widget *widget,overlay_widgets){
        if (widget->rect.contains(e->pos())){
            if (last_pressed_widget == widget && viewer != nullptr)
                widget->clicked();

            result = true;
        }
    }

    last_pressed_widget = nullptr;
    return result;
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


    stacked_widget->setMouseTracking(true);
    scroll_area->setMouseTracking(true);
    label->setMouseTracking(true);
    setMouseTracking(true);
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
            overlay->repaint();
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
    if (viewer != nullptr)
        viewer->resize(scroll_area->width()-1,scroll_area->height()-1,scale_type);
}

void MediaViewer::set_scale(int scale)
{
    scale_type = (Viewer::SCALE_TYPE)scale;
    update_scale();
}

void MediaViewer::set_focus()
{
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
    if (overlay->mouseMoved(e)) return;

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

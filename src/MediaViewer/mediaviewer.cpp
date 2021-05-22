#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QAction>
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

Overlay_Button::Overlay_Button(QByteArray icon_path,bool *active_ptr,QString tooltip,Overlay *parent,bool toggleable,QByteArray icon_off_path) : QObject(parent),toggleable(toggleable),active(active_ptr)
{
    if (!icon_path.isEmpty()){
        icon = QImage(icon_path);
        icon = icon.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
    }

    if (toggleable && !icon_off_path.isEmpty()){
        icon_off = QImage(icon_off_path);
        icon_off = icon_off.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

        connect(this,&Overlay_Button::clicked,this,&Overlay_Button::toggle);
        connect(this,&Overlay_Button::toggle_changed,this,&Overlay_Button::request_redraw);
    } else if (toggleable && !icon.isNull()){

        icon_off = icon;

        for(int y = 0; y < icon_off.height(); y++)
            for(int x= 0; x < icon_off.width(); x++)
                icon_off.setPixelColor(x,y,QColor(155,155,155,icon_off.pixelColor(x,y).alpha()));

        connect(this,&Overlay_Button::clicked,this,&Overlay_Button::toggle);
        connect(this,&Overlay_Button::toggle_changed,this,&Overlay_Button::request_redraw);
    }

    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));

    this->tooltip = tooltip;
}

Overlay_Text::Overlay_Text(QString tooltip,Overlay *parent): QObject(parent)
{
    this->tooltip = tooltip;
    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

Overlay_Spacer::Overlay_Spacer(int *available_space) : unused_space(available_space){}

Overlay_Progress_Bar::Overlay_Progress_Bar(int *available_space,Overlay *parent) : QObject(parent),Overlay_Spacer(available_space)
{
    connect(this,&Overlay_Progress_Bar::clicked,this,&Overlay_Progress_Bar::progress_change_request);
    connect(this,SIGNAL(request_redraw()),parent,SLOT(repaint()));
}

Overlay_Combo_Box::Overlay_Combo_Box(bool *active_ptr,QString tooltip,Overlay *parent) : Overlay_Button("",active_ptr,tooltip,parent)
{
    connect(this,&Overlay_Combo_Box::clicked,this,&Overlay_Combo_Box::toggle_drop_down);
}

Overlay_Volume_Button::Overlay_Volume_Button(bool *active_ptr,Overlay *parent): Overlay_Button(":/data/icons/volume_med",active_ptr,"Volume",parent)
{
    icon_low = QImage(":/data/icons/volume_low");
    icon_mute = QImage(":/data/icons/mute.png");

    icon_low = icon_low.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    icon_mute = icon_mute.scaled(
                QSize(width,height),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation);

    icon_med = icon;

    connect(this,&Overlay_Volume_Button::clicked,this,&Overlay_Volume_Button::volume_change_request);
    connect(this,&Overlay_Volume_Button::clicked,this,&Overlay_Volume_Button::toggle_popup);
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
    Overlay_Button *lock_button = new Overlay_Button(":/data/icons/lock.png",nullptr,"Lock Overlay",this,true);
    Overlay_Button *zoom_out_button = new Overlay_Button(":/data/icons/zoom_out.png",&show_zoom,"Zoom out",this);
    Overlay_Button *zoom_in_button = new Overlay_Button(":/data/icons/zoom_in.png",&show_zoom,"Zoom in",this);

    Overlay_Button *prev_page_button = new Overlay_Button(":/data/icons/prev.png",&show_page,"Previous page",this);
    Overlay_Button *next_page_button = new Overlay_Button(":/data/icons/next.png",&show_page,"Next page",this);

    Overlay_Text *info_text = new Overlay_Text("",this);
    Overlay_Text *mime_text = new Overlay_Text("Mime type",this);
    Overlay_Text *time_text = new Overlay_Text("Import time",this);
    Overlay_Text *size_text = new Overlay_Text("File size",this);

    Overlay_Spacer *spacer = new Overlay_Spacer(&main_bar.unused_space);
    Overlay_Combo_Box *auto_scale = new Overlay_Combo_Box(&show_zoom,"Scale",this);

    auto_scale->entries.append({"Fit Both",Viewer::SCALE_TYPE::BOTH});
    auto_scale->entries.append({"Fit Height",Viewer::SCALE_TYPE::HEIGHT});
    auto_scale->entries.append({"Fit Width",Viewer::SCALE_TYPE::WIDTH});
    auto_scale->entries.append({"Original size",Viewer::SCALE_TYPE::ORIGINAL});

    connect(lock_button,&Overlay_Button::clicked,this,&Overlay::toggle_lock);
    connect(parent,&MediaViewer::lock_overlay,lock_button,&Overlay_Button::clicked);

    connect(zoom_out_button,&Overlay_Button::clicked,parent,&MediaViewer::zoom_out);
    connect(zoom_in_button,&Overlay_Button::clicked,parent,&MediaViewer::zoom_in);

    connect(prev_page_button,&Overlay_Button::clicked,parent,&MediaViewer::prev_page);
    connect(next_page_button,&Overlay_Button::clicked,parent,&MediaViewer::next_page);

    connect(this,&Overlay::update_file_info,info_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_mime_info,mime_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_time_info,time_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_size_info,size_text,&Overlay_Text::set_text);

    connect(auto_scale,&Overlay_Combo_Box::entry_changed,parent,&MediaViewer::set_scale);


    main_bar.widgets.append(zoom_out_button);
    main_bar.widgets.append(zoom_in_button);
    main_bar.widgets.append(prev_page_button);
    main_bar.widgets.append(next_page_button);
    main_bar.widgets.append(auto_scale);
    main_bar.widgets.append(lock_button);

    main_bar.widgets.append(spacer);
    main_bar.widgets.append(info_text);
    main_bar.widgets.append(size_text);
    main_bar.widgets.append(mime_text);
    main_bar.widgets.append(time_text);

    // playback bar
    Overlay_Button *pause_button = new Overlay_Button(":/data/icons/pause.png",nullptr,"Pause",this,true,":/data/icons/next.png");
    Overlay_Text *position_text = new Overlay_Text("File Position",this);
    Overlay_Text *duration_text = new Overlay_Text("File Duration",this);
    Overlay_Progress_Bar *prog_bar = new Overlay_Progress_Bar(&playback_bar.unused_space,this);
    Overlay_Volume_Button *vol_button = new Overlay_Volume_Button(nullptr,this);

    connect(this,&Overlay::update_playback_position,position_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_playback_duration,duration_text,&Overlay_Text::set_text);
    connect(this,&Overlay::update_playback_progress,prog_bar,&Overlay_Progress_Bar::set_progress);
    connect(this,&Overlay::update_playback_state,pause_button,&Overlay_Button::set_toggle_state);
    connect(this,&Overlay::update_playback_volume,vol_button,&Overlay_Volume_Button::set_volume);
    connect(this,&Overlay::update_playback_has_audio,vol_button,&Overlay_Volume_Button::set_has_audio);
    connect(this,&Overlay::update_playback_mute_state,vol_button,&Overlay_Volume_Button::set_mute_state);
    connect(pause_button,&Overlay_Button::clicked,this,&Overlay::pause);

    connect(prog_bar,&Overlay_Progress_Bar::change_progress,this,&Overlay::change_progress);
    connect(vol_button,&Overlay_Volume_Button::change_volume,this,&Overlay::change_volume);
    connect(vol_button,&Overlay_Volume_Button::change_mute,this,&Overlay::change_muted);

    playback_bar.widgets.append(pause_button);
    playback_bar.widgets.append(position_text);
    playback_bar.widgets.append(prog_bar);
    playback_bar.widgets.append(duration_text);
    playback_bar.widgets.append(vol_button);

    auto_hide_timer.setSingleShot(true);
    connect(&auto_hide_timer, &QTimer::timeout, this, &Overlay::set_hidden);

    setAttribute(Qt::WA_TransparentForMouseEvents);

    raise();

}

Overlay::~Overlay()
{
    foreach(Overlay_Bar *bar,overlay_bars)
        foreach(Overlay_Widget *widget,bar->widgets)
            delete widget;
}

Overlay_Widget::~Overlay_Widget(){};

bool Overlay_Widget::check_hover(QMouseEvent *e)
{
    if (rect.contains(e->pos()))
        return true;
    return false;
}


void Overlay::paintEvent(QPaintEvent *)
{
    if (state == HIDDEN)
        return;

    QPainter p(this);

    p.setRenderHints(QPainter::Antialiasing);

    p.setPen(Qt::white);

    for	(int i = overlay_bars.size()-1;i >= 0;i--)
        overlay_bars[i]->draw(*viewer,p);
}

void Overlay::Overlay_Bar::draw(Viewer *viewer, QPainter &p)
{
    if (active){
        p.fillRect(rect,bg);
        if (viewer != nullptr){
            int used_space = 0;
            int offset = left_margin;
            int widget_bottom = rect.bottom()-bottom_margin;

            foreach(Overlay_Widget *widget,widgets)
                used_space += widget->requested_width(p);

            unused_space = rect.width()-used_space;

            foreach(Overlay_Widget *widget,widgets)
                offset += widget->draw(p,offset,widget_bottom);

        }
    }

}


void Overlay::resizeEvent(QResizeEvent *e)
{
    int bottom = rect().bottom()+1;
    foreach(Overlay_Bar *bar,overlay_bars){
        bar->rect = QRect(0,bottom-bar->height,rect().width(),bar->height);
        bottom -= bar->height;
    }

    QWidget::resizeEvent(e);
}

bool Overlay_Widget::activate_hover(QMouseEvent *e)
{
    if (!tooltip.isEmpty() && rect.contains(e->pos()))
        QToolTip::showText(e->globalPos(),tooltip);
    return false;
}

bool Overlay_Button::activate_hover(QMouseEvent *e)
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
   return highlighed;
}

bool Overlay_Progress_Bar::activate_hover(QMouseEvent *e)
{
    if (rect.contains(e->pos())){
        hover_progress = (e->pos().x()-rect.left())/(double)rect.width();
        return true;
    }

    return false;

}

bool Overlay_Combo_Box::activate_hover(QMouseEvent *e)
{
    bool inital_status = highlighed;
    int last_highlighted_entry = highlighted_entry;

    if (entries.empty())
        return false;

    if (rect.contains(e->pos())){
        highlighed = true;
        if (!display_dropdown)
            QToolTip::showText(e->globalPos(),tooltip);

    } else {
        highlighed = false;
    }


    if (display_dropdown){
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

    }

    if (inital_status != highlighed ||
            last_highlighted_entry != highlighted_entry){
        emit request_redraw();
    }

    return (highlighted_entry != -1 || highlighed);
}
bool Overlay_Volume_Button::check_hover(QMouseEvent *e)
{
    if (Overlay_Widget::check_hover(e))
        return true;

    return popup_visible && popup_rect.contains(e->pos());
}
bool Overlay_Combo_Box::check_hover(QMouseEvent *e)
{
    if (Overlay_Widget::check_hover(e))
        return true;

    if (display_dropdown)
        for(int i = 0;i < dropdownrect.length();i++)
            if (dropdownrect[i].contains(e->pos()))
                return true;

    return false;
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
    if ((active == nullptr || (*active)) && !entries.isEmpty())
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

        if (!toggleable || is_toggled){
            p.drawImage(rect,icon.scaled(width,height));
        } else {
            p.drawImage(rect,icon_off.scaled(width,height));
        }


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

        }

        return used_width+3;

    } else {
        return 0;
    }

}

int Overlay_Volume_Button::draw(QPainter &p,int x,int y)
{
    if (popup_visible){
        QBrush bg(QColor(80,80,80,255));
        popup_rect = QRect(x+rect.width()/4,y-rect.width()-3-100,rect.width()/2,100);
        QRect volume_rect = popup_rect.adjusted(0,100-volume_level,0,0);


        p.fillRect(popup_rect,QBrush(QColor(0,0,0,255)));
        p.fillRect(volume_rect,bg);
        p.drawRect(popup_rect);
    }

    if (has_audio){
        return Overlay_Button::draw(p,x,y);
    } else {
        if (active == nullptr || (*active)){
            rect = QRect(x,y-width,width,height);
            p.drawImage(rect,icon_mute.scaled(width,height));
            return width+3;
        } else {
            return 0;
        }
    }

}

bool Overlay_Volume_Button::activate_hover(QMouseEvent *e)
{
    if (popup_visible && popup_rect.contains(e->pos())){
        hover_volume = (popup_rect.bottom()-e->pos().y())/(double)popup_rect.height();
        return true;
    } else {
        hover_volume = -1;
    }

    return Overlay_Button::activate_hover(e);
}


void Overlay_Progress_Bar::set_progress(int new_progress,int max)
{
    progress = double(new_progress)/max*100;
    emit request_redraw();
}

void Overlay_Combo_Box::toggle_drop_down()
{
    if (entries.isEmpty())
        return;

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
    if (state != HIDDEN && !locked){
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

        if ((playback_bar.active = (*viewer)->controller != nullptr)){
            Playback_Controller *controller = (*viewer)->controller;
            connect(controller,&Playback_Controller::duration_changed,this,&Overlay::update_playback_duration);
            connect(controller,&Playback_Controller::position_changed,this,&Overlay::update_playback_position);
            connect(controller,&Playback_Controller::playback_state_changed,this,&Overlay::update_playback_state);
            connect(controller,&Playback_Controller::update_progress,this,&Overlay::update_playback_progress);
            connect(controller,&Playback_Controller::volume_changed,this,&Overlay::update_playback_volume);

            connect(this,&Overlay::pause,controller,&Playback_Controller::pause);
            connect(this,&Overlay::rewind,controller,&Playback_Controller::rewind);
            connect(this,&Overlay::fast_forward,controller,&Playback_Controller::fast_forward);
            connect(this,&Overlay::change_progress,controller,&Playback_Controller::set_progress);
            connect(this,&Overlay::change_volume,controller,&Playback_Controller::set_volume);
            connect(this,&Overlay::change_muted,controller,&Playback_Controller::set_mute);

            emit update_playback_duration(controller->duration());
            emit update_playback_position(controller->position());
            emit update_playback_state(controller->pause_state());
            emit update_playback_has_audio(controller->has_audio());
        }

    } else {
        playback_bar.active = false;
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
    foreach(Overlay_Bar *bar,overlay_bars)
        foreach(Overlay_Widget *widget,bar->widgets)
            if (widget->activate_hover(e))
                return true;

    return false;
}

bool Overlay::mouseClicked(QMouseEvent *e)
{

    foreach(Overlay_Bar *bar,overlay_bars){
        foreach(Overlay_Widget *widget,bar->widgets){
            if (widget->check_hover(e)){
                last_pressed_widget = widget;
                return true;
            }
        }
    }

    return false;
}

bool Overlay::mouseReleased(QMouseEvent *e)
{

    foreach(Overlay_Bar *bar,overlay_bars){
        foreach(Overlay_Widget *widget,bar->widgets){
            if (widget->check_hover(e)){
                if (last_pressed_widget == widget && viewer != nullptr){
                    widget->clicked();
                    return true;
                }
            }
        }
    }

    last_pressed_widget = nullptr;
    return false;
}

void Overlay::toggle_lock()
{
    locked = !locked;
    if (locked)
        set_visible();
    else if (auto_hide)
        set_hidden();

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
    video_player->setMouseTracking(true);

    setMouseTracking(true);

    QAction *pause = new QAction("pause",this);
    QAction *seek_right = new QAction("seek right",this);
    QAction *seek_left = new QAction("seek left",this);

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

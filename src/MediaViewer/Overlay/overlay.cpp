#include <QMouseEvent>
#include <QDateTime>
#include <QLocale>

#include <pyros.h>

#include "../playback_controller.h"
#include "../mediaviewer.h"
#include "../../configtab.h"

#include "overlay.h"
#include "overlay_widget.h"
#include "overlay_text.h"
#include "overlay_button.h"
#include "overlay_spacer.h"
#include "overlay_combo_box.h"
#include "overlay_progress_bar.h"
#include "overlay_volume_button.h"


using ct = configtab;

Overlay::Overlay(Viewer **viewer,MediaViewer *parent) : QWidget(parent),viewer(viewer)
{
    Overlay_Button *overlay_next_button = new Overlay_Button(":/data/icons/right_arrow.png",nullptr,"Next file",this);
    Overlay_Button *overlay_prev_button = new Overlay_Button(":/data/icons/left_arrow.png",nullptr,"Prev file",this);

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

    Overlay_Text *overlay_file_count = new Overlay_Text("File count",this);
    Overlay_Button *overlay_delete_button = new Overlay_Button(":/data/icons/trash.png",&parent->files_deletable,"Delete file",this);

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

    connect(overlay_next_button,&Overlay_Button::clicked,parent,&MediaViewer::next_file);
    connect(overlay_prev_button,&Overlay_Button::clicked,parent,&MediaViewer::prev_file);

    connect(parent,&MediaViewer::update_file_count,overlay_file_count,&Overlay_Text::set_text);
    connect(auto_scale,&Overlay_Combo_Box::entry_changed,parent,&MediaViewer::set_scale);
    connect(overlay_delete_button,&Overlay_Button::clicked,parent,&MediaViewer::delete_file);

    main_bar.widgets.append(overlay_prev_button);
    main_bar.widgets.append(overlay_next_button);

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
    main_bar.widgets.append(overlay_file_count);
    main_bar.widgets.append(overlay_delete_button);

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
    connect(this,&Overlay::toggle_playback_mute_state,vol_button,&Overlay_Volume_Button::toggle_mute);
    connect(pause_button,&Overlay_Button::clicked,this,&Overlay::pause);

    connect(prog_bar,&Overlay_Progress_Bar::change_progress,this,&Overlay::change_progress);
    connect(vol_button,&Overlay_Volume_Button::change_volume,this,&Overlay::change_volume);

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


void Overlay::set_visible()
{
    if (state != DISPLAYED){
        state = DISPLAYED;
        repaint();
    }

    if (auto_hide){
        if (auto_hide_timer.isActive())
            auto_hide_timer.stop();

        auto_hide_timer.start(1500);
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
    if (viewer != nullptr && file != nullptr){
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
            connect(controller,&Playback_Controller::has_audio_changed,this,&Overlay::update_playback_has_audio);

            connect(this,&Overlay::pause,controller,&Playback_Controller::pause);
            connect(this,&Overlay::rewind,controller,&Playback_Controller::rewind);
            connect(this,&Overlay::fast_forward,controller,&Playback_Controller::fast_forward);
            connect(this,&Overlay::change_progress,controller,&Playback_Controller::set_progress);
            connect(this,&Overlay::change_volume,controller,&Playback_Controller::set_volume);

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
    set_visible();
    foreach(Overlay_Bar *bar,overlay_bars)
        foreach(Overlay_Widget *widget,bar->widgets)
            if (widget->activate_hover(e))
                return true;

    return false;
}

bool Overlay::mouseClicked(QMouseEvent *e)
{
    bool result = false;

    foreach(Overlay_Bar *bar,overlay_bars){
        foreach(Overlay_Widget *widget,bar->widgets){
            if ( !result && widget->check_hover(e)){
                last_pressed_widget = widget;
                result = true;
            } else {
                widget->unselected();
            }
        }
    }

    return result;
}

bool Overlay::mouseReleased(QMouseEvent *e)
{

    foreach(Overlay_Bar *bar,overlay_bars){
        foreach(Overlay_Widget *widget,bar->widgets){
            if (widget->check_hover(e)){
                if (last_pressed_widget == widget && viewer != nullptr){
                    if (e->button() == Qt::MiddleButton)
                        widget->middle_button_clicked();
                    else
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

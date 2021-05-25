#include <QTime>

#include "../configtab.h"
#include "video_viewer.h"

using ct = configtab;

Video_Viewer::Video_Viewer(mpv_widget *mpv): Viewer(nullptr),m_mpv(mpv)
{
    controller = new Mpv_Controller(m_mpv);

}

Video_Viewer::~Video_Viewer()
{
    m_mpv->stop();
    delete controller;
}

Mpv_Controller::Mpv_Controller(mpv_widget *mpv) : Playback_Controller(mpv),mpv(mpv)
{
    connect(mpv,&mpv_widget::position_changed,this,&Mpv_Controller::set_position);
    connect(mpv,&mpv_widget::remaining_changed,this,&Mpv_Controller::set_remaining);
    connect(mpv,&mpv_widget::duration_changed,this,&Mpv_Controller::set_duration);
    connect(mpv,&mpv_widget::playback_state,this,&Mpv_Controller::playback_state_changed);
    connect(mpv,&mpv_widget::volume_changed,this,&Mpv_Controller::volume_changed);
    connect(mpv,&mpv_widget::has_audio,this,&Mpv_Controller::has_audio_changed);

}

QString Mpv_Controller::duration()
{
    if (!ct::setting_value(ct::SHOW_REMAINING_TIME).toBool())
        return milliToStr(m_duration*1000);
    else
        return milliToStr(m_remaining*1000);
}

void Mpv_Controller::set_duration(double dur)
{
    m_duration = dur;
    if (dur*1000 < 2000)
        show_milliseconds = true;
    else
        show_milliseconds = false;

    if (!ct::setting_value(ct::SHOW_REMAINING_TIME).toBool())
        emit duration_changed(duration());
}

void Mpv_Controller::set_remaining(double rem)
{
    m_remaining = rem;
    if (ct::setting_value(ct::SHOW_REMAINING_TIME).toBool())
        emit duration_changed(duration());
}

QString Mpv_Controller::position()
{
    return milliToStr(m_position*1000);
}

bool Mpv_Controller::pause_state()
{
    return mpv->is_paused();
}

void Mpv_Controller::set_position(double pos)
{
    m_position = pos;
    emit position_changed(position());
    emit update_progress(m_position*1000,m_duration*1000);
}

void Mpv_Controller::fast_forward()
{
    mpv->fast_forward();
}

void Mpv_Controller::rewind()
{
    mpv->rewind();
}

void Mpv_Controller::pause()
{
    mpv->toggle_playback();
}

void Mpv_Controller::set_progress(double progress)
{
    mpv->set_progress(progress);
}

void Mpv_Controller::set_volume(double volume)
{
    mpv->set_volume(volume);
}


bool Mpv_Controller::has_audio()
{
    return mpv->check_if_has_audio();
}

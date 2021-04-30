#include <QTime>

#include "video_viewer.h"

Video_Viewer::Video_Viewer(mpv_widget *mpv): Viewer(nullptr),m_mpv(mpv)
{
    controller = new Mpv_Controller(m_mpv);

}

Video_Viewer::~Video_Viewer()
{
    m_mpv->stop();
}

Mpv_Controller::Mpv_Controller(mpv_widget *mpv) : Playback_Controller(mpv),mpv(mpv)
{
    connect(mpv,&mpv_widget::position_changed,this,&Mpv_Controller::set_position);
    connect(mpv,&mpv_widget::duration_changed,this,&Mpv_Controller::set_duration);

}

QString Mpv_Controller::duration()
{
    return milliToStr(m_duration*1000);
}

void Mpv_Controller::set_duration(double dur)
{
    m_duration = dur;
    emit duration_changed(duration());
}

QString Mpv_Controller::position()
{
    return milliToStr(m_position*1000);
}

void Mpv_Controller::set_position(double pos)
{
    m_position = pos;
    emit position_changed(position());
    emit update_progress(m_position*1000,m_duration*1000);
}

void Mpv_Controller::fast_forward()
{
    mpv->quick_fast_forward();
}

void Mpv_Controller::rewind()
{
    mpv->quick_rewind();
}

void Mpv_Controller::pause()
{
    mpv->toggle_playback();
}

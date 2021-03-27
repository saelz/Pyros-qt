#include "video_viewer.h"

Video_Viewer::Video_Viewer(mpv_widget *mpv): Viewer(nullptr),m_mpv(mpv){}

Video_Viewer::~Video_Viewer()
{
    m_mpv->stop();
}

#ifndef VIDEO_VIEWER_H
#define VIDEO_VIEWER_H

#include "viewer.h"
#include "mpv_widget.h"

class Video_Viewer : public Viewer
{
    mpv_widget *m_mpv = nullptr;
public:
    Video_Viewer(mpv_widget *mpv);

    ~Video_Viewer();

    inline void set_file(char *path) override
    {
        m_mpv->set_file(path);
    }
};

#endif // VIDEO_VIEWER_H

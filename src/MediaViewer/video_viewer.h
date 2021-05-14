#ifndef VIDEO_VIEWER_H
#define VIDEO_VIEWER_H

#include "viewer.h"
#include "mpv_widget.h"
#include "playback_controller.h"

class Mpv_Controller : public Playback_Controller{
    mpv_widget *mpv;
public:
    double m_duration = 0;
    double m_position = 0;
    Mpv_Controller(mpv_widget *mpv);
    QString duration() override;
    QString position() override;
    bool pause_state() override;

public slots:
    void set_duration(double dur);
    void set_position(double dur);

    void fast_forward() override;
    void rewind() override;
    void pause() override;
};

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

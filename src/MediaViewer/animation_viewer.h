#ifndef ANIMATION_VIEWER_H
#define ANIMATION_VIEWER_H

#include "image_viewer.h"
#include "playback_controller.h"

#include <QTimer>

class QImageReader;

class Animation_Controller;

class Animation_Viewer :  public Image_Viewer{
    QSize orignal_size;
public:
    Animation_Viewer(QLabel *label);
    ~Animation_Viewer();

    struct Frame{
        Frame(QPixmap pixmap,int delay = 0):pixmap(pixmap),delay(delay){};
        Frame(){};
        QPixmap pixmap;
        int delay;
    };

    void set_file(char *path) override;
    void set_frame(int frame_number);
    QVector<Frame> frames;
    int current_frame = 0;
};

class Animation_Controller : public Playback_Controller{
public:

    bool is_paused = false;
    int remaining_pause_time = 0;

    Animation_Viewer *viewer;
    Animation_Controller(Animation_Viewer *viewer);
    QString duration() override;
    QString position() override;
    inline bool pause_state() override {return is_paused;};

public slots:
    void fast_forward() override;
    void rewind() override;
    void pause() override;
    void set_progress(double progress) override;
    void update_progress_stat();

private slots:
    QTimer frame_timer;
};

#endif // ANIMATION_VIEWER_H

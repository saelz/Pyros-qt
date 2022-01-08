#include "animation_viewer.h"

#include <QImageReader>
#include <QTimer>

Animation_Viewer::Animation_Viewer(QLabel *label) : Image_Viewer(label)
{
}

Animation_Viewer::~Animation_Viewer()
{
    delete controller;
}

void Animation_Viewer::set_file(char *path)
{
    QImageReader img_reader(path);
    QImage img;
    Frame frame;

    m_img.convertFromImage(img_reader.read());
    frames.append(Frame(m_img,img_reader.nextImageDelay()));

    while(!(img = img_reader.read()).isNull()){
        QPixmap pix;
        pix.convertFromImage(img);

        frames.append(Frame(pix,img_reader.nextImageDelay()));
    }

    if (frames.count() > 1){
        controller = new Animation_Controller(this);
        controller->fast_forward();
    }
}

void Animation_Viewer::set_frame(int frame_number)
{
    Frame frame;

    if (frame_number >= frames.count())
        frame_number = 0;
    else if (frame_number < 0)
        frame_number = frames.count()-1;

    frame = frames.at(frame_number);
    m_img = frame.pixmap;
    update_size();
    current_frame = frame_number;

}

Animation_Controller::Animation_Controller(Animation_Viewer *viewer) : Playback_Controller(nullptr),viewer(viewer)
{
    frame_timer.setSingleShot(true);
    connect(&frame_timer, &QTimer::timeout, this, &Animation_Controller::fast_forward);
}

void Animation_Controller::fast_forward()
{
    viewer->set_frame(viewer->current_frame);
    update_progress_stat();

    if (!is_paused)
        frame_timer.start(viewer->frames.at(viewer->current_frame).delay);
    else
        remaining_pause_time = 0;

    viewer->current_frame++;
}

QString Animation_Controller::duration()
{
    return QString::number(viewer->frames.count());
}

QString Animation_Controller::position()
{
    return QString::number(viewer->current_frame+1);
}
void Animation_Controller::rewind()
{
    viewer->current_frame = viewer->current_frame-2;
    fast_forward();
}

void Animation_Controller::pause()
{
    is_paused = !is_paused;

    if (is_paused){
        remaining_pause_time = frame_timer.remainingTime();
        frame_timer.stop();
    } else{
        frame_timer.start(remaining_pause_time);
    }

    update_progress_stat();

}

void Animation_Controller::set_progress(double progress)
{
    viewer->set_frame((viewer->frames.count()*progress));
    update_progress_stat();
}

void Animation_Controller::update_progress_stat()
{
    emit update_progress(viewer->current_frame,viewer->frames.count()-1);
    emit position_changed(position());
    emit playback_state_changed(!is_paused);
}

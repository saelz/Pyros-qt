#include <QMovie>
#include <QLabel>

#include "movie_viewer.h"

Movie_Viewer::Movie_Viewer(QLabel *label) : Image_Viewer(label){}

Movie_Viewer::~Movie_Viewer()
{
    if (movie != nullptr)
        delete movie;
}

void Movie_Viewer::set_file(char *path){
    movie = new QMovie(path);
    movie->setCacheMode(QMovie::CacheAll);

    Image_Viewer::m_label->setMovie(movie);
    movie->start();
    orignal_size = movie->currentImage().size();
    controller = new Movie_Controller(movie);

    emit controller->duration_changed(controller->duration());
    ((Movie_Controller*)controller)->playback_changed(movie->state());
}

bool Movie_Controller::pause_state()
{
    if (movie->state() == QMovie::NotRunning || movie->state() == QMovie::Paused)
        return false;
    else
        return true;

}

void Movie_Viewer::set_size(QSize newsize){
    if (movie == nullptr)
        return;

    scaled_size = newsize;

    movie->setScaledSize(newsize);

    // redraw frame
    int current_frame = movie->currentFrameNumber();
    if (movie->jumpToNextFrame())
        movie->jumpToFrame(current_frame);
    else if (movie->frameCount() == 1)
        controller->pause();
}

Movie_Controller::Movie_Controller(QMovie *movie) : Playback_Controller(movie),movie(movie)
{
    connect(movie,&QMovie::frameChanged,this,&Movie_Controller::set_position);
    connect(movie,&QMovie::stateChanged,this,&Movie_Controller::playback_changed);
}

QString Movie_Controller::duration()
{
    return QString::number(movie->frameCount());
}

QString Movie_Controller::position()
{
    return QString::number(movie->currentFrameNumber());
}

QString Movie_Viewer::get_info()
{
    if (movie == nullptr)
        return "";

    return Image_Viewer::get_info();
}

void Movie_Controller::set_position(int frame)
{
    emit update_progress(frame,movie->frameCount());
    emit position_changed(position());
}

void Movie_Controller::fast_forward()
{
    movie->jumpToNextFrame();
}

void Movie_Controller::rewind()
{
    if (movie->currentFrameNumber() > 0)
        movie->jumpToFrame(movie->currentFrameNumber()-1);
    else
        movie->jumpToFrame(movie->frameCount()-1);
}

void Movie_Controller::pause()
{
    if (movie->state() == QMovie::NotRunning || movie->state() == QMovie::Paused)
        movie->setPaused(false);
    else
        movie->setPaused(true);
}

void Movie_Controller::playback_changed(QMovie::MovieState state)
{
    if (state == QMovie::NotRunning || state == QMovie::Paused)
        emit playback_state_changed(false);
    else
        emit playback_state_changed(true);
}

void Movie_Controller::set_progress(double progress)
{
    movie->jumpToFrame(movie->frameCount()*progress);
}

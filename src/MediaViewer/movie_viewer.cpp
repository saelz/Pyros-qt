#include <QMovie>
#include <QLabel>

#include "movie_viewer.h"

Movie_Viewer::Movie_Viewer(QLabel *label) : Image_Viewer(label)
{
}

Movie_Viewer::~Movie_Viewer()
{
    if (movie != nullptr)
        delete movie;
}

void Movie_Viewer::set_file(char *path){
    movie = new QMovie(path);

    Image_Viewer::m_label->setMovie(movie);
    movie->start();
    orignal_size = movie->currentImage().size();
    controller = new Movie_Controller(movie);
    emit controller->duration_changed(controller->duration());
}

void Movie_Viewer::set_size(QSize newsize){
    if (movie == nullptr)
        return;

    scaled_size = newsize;

    movie->stop();
    movie->setScaledSize(newsize);
    movie->start();
}

Movie_Controller::Movie_Controller(QMovie *movie) : Playback_Controller(movie),movie(movie)
{
    connect(movie,&QMovie::frameChanged,this,&Movie_Controller::set_position);
}

QString Movie_Controller::duration()
{
    return milliToStr(movie->frameCount()*movie->nextFrameDelay());
}

QString Movie_Controller::position()
{
    return milliToStr(movie->currentFrameNumber()*movie->nextFrameDelay());
}

QString Movie_Viewer::get_info()
{
    if (movie == nullptr)
        return "";
    return "Frames:"+QString::number(movie->frameCount())+
            " "+Image_Viewer::get_info();
}

void Movie_Controller::set_position(int frame)
{
    emit update_progress(frame,movie->frameCount());
    emit position_changed(milliToStr(frame*movie->nextFrameDelay()));
}

void Movie_Controller::fast_forward()
{
    movie->jumpToNextFrame();
}

void Movie_Controller::rewind()
{
}

void Movie_Controller::pause()
{
    if (movie->state() == QMovie::NotRunning || movie->state() == QMovie::Paused)
        movie->setPaused(false);
    else
        movie->setPaused(true);
}

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

    m_label->setMovie(movie);
    movie->start();
    orignal_size = movie->currentImage().size();
}

void Movie_Viewer::set_size(QSize newsize){
    if (movie == nullptr)
        return;

    movie->stop();
    movie->setScaledSize(newsize);
    movie->start();
}

QString Movie_Viewer::get_info()
{
    if (movie == nullptr)
        return "";
    return "Frames:"+QString::number(movie->frameCount())+
            " "+Image_Viewer::get_info();
}

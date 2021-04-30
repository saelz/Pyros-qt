#ifndef MOVIE_VIEWER_H
#define MOVIE_VIEWER_H

#include "image_viewer.h"
#include "playback_controller.h"

class QMovie;

class Movie_Controller : public Playback_Controller{
public:
    QMovie *movie;
    Movie_Controller(QMovie *movie);
    QString duration() override;
    QString position() override;
private slots:
    void set_position(int frame);
};

class Movie_Viewer :  public Image_Viewer{
    QMovie *movie = nullptr;
    QSize orignal_size;
public:
    Movie_Viewer(QLabel *label);

    ~Movie_Viewer();

    void set_file(char *path) override;

    inline QSize size() override {return orignal_size;}

    void set_size(QSize newsize) override;


    QString get_info() override;
};


#endif // MOVIE_VIEWER_H

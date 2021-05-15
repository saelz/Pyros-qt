#ifndef MOVIE_VIEWER_H
#define MOVIE_VIEWER_H

#include "image_viewer.h"
#include "playback_controller.h"
#include <QMovie>


class Movie_Controller : public Playback_Controller{
public:
    QMovie *movie;
    Movie_Controller(QMovie *movie);
    QString duration() override;
    QString position() override;
    bool pause_state() override;

public slots:
    void fast_forward() override;
    void rewind() override;
    void pause() override;
    void set_progress(double progress) override;
    void playback_changed(QMovie::MovieState state);

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

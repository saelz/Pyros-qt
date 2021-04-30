#ifndef VIEWER_H
#define VIEWER_H

#include <QSize>
#include <QString>


class QString;
class QLabel;
class Playback_Controller;

class Viewer{
public:
    enum SCALE_TYPE{
        HEIGHT,
        WIDTH,
        BOTH,
        ORIGINAL,
    };

protected:
    QLabel *m_label;
    int boundry_width = 0;
    int boundry_height = 0;
    SCALE_TYPE scale_type =BOTH;
public:
    Viewer(QLabel *label);
    Viewer();
    virtual ~Viewer();

    Playback_Controller *controller = nullptr;

    virtual void set_file(char *filepath) = 0;
    virtual void resize(int width,int height,SCALE_TYPE scale);

    virtual inline void update_size(){};
    virtual inline void zoom_in(){};
    virtual inline void zoom_out(){};
    virtual inline QSize current_size() {return QSize();};

    virtual inline bool next_page(){return  false;};
    virtual inline bool prev_page(){return  false;};

    virtual inline QString get_info() {return "";};

    virtual inline bool resizable(){return false;}
    virtual inline bool scaleing(){return false;}
    virtual inline bool multi_paged(){return false;}
    virtual inline bool always_show_vertical_scrollbar(){return false;}

};

#endif // VIEWER_H

#ifndef VIEWER_H
#define VIEWER_H

#include "mediaviewer.h"

class QString;
//class QLabel;

class Viewer{
protected:
    QLabel *m_label;
    int boundry_width = 0;
    int boundry_height = 0;
    MediaViewer::SCALE_TYPE scale_type = MediaViewer::BOTH;
public:
    Viewer(QLabel *label);
    Viewer();
    virtual ~Viewer();

    virtual void set_file(char *filepath) = 0;
    virtual void resize(int width,int height,MediaViewer::SCALE_TYPE scale);

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

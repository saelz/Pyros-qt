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
    inline virtual void resize(int width,int height,MediaViewer::SCALE_TYPE scale)
    {
        if (boundry_height != height ||
                boundry_width != width ||
                scale_type != scale){

            boundry_height = height;
            boundry_width = width;
            scale_type = scale;
            update_size();
        }
    }

    virtual inline void update_size(){};
    virtual inline void zoom_in(){};
    virtual inline void zoom_out(){};

    virtual inline void next_page(){};
    virtual inline void prev_page(){};

    virtual inline QString get_info(){return "";}

    virtual inline bool resizable(){return false;}
    virtual inline bool scaleing(){return false;}
    virtual inline bool multi_paged(){return false;}
    virtual inline bool always_show_vertical_scrollbar(){return false;}
};

#endif // VIEWER_H

#ifndef TEXT_VIEWER_H
#define TEXT_VIEWER_H

#include "viewer.h"

class Text_Viewer : public Viewer{
    int font_size = 12;
public:
    Text_Viewer(QLabel *label);

    ~Text_Viewer();

    void set_file(char *path) override;

    void zoom_in() override;

    void zoom_out() override;

    QString get_info() override;

    inline bool resizable() override{return true;}
    inline bool always_show_vertical_scrollbar() override{return true;}

};



#endif // TEXT_VIEWER_H

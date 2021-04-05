#ifndef UNSUPPORTED_VIEWER_H
#define UNSUPPORTED_VIEWER_H

#include "text_viewer.h"

class Unsupported_Viewer : public Text_Viewer{
public:
    Unsupported_Viewer(QLabel*label);
    void set_file(char *path) override;

    inline QString get_info() override{ return "";}
    inline bool always_show_vertical_scrollbar() override{return false;}
};

#endif // UNSUPPORTED_VIEWER_H

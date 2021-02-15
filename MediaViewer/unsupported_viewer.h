#ifndef UNSUPPORTED_VIEWER_H
#define UNSUPPORTED_VIEWER_H

#include "text_viewer.h"

class Unsupported_Viewer : public Text_Viewer{
public:
    Unsupported_Viewer(QLabel*label);
    void set_file(char *path) override;

    inline QString get_info() override{ return "";}
};

#endif // UNSUPPORTED_VIEWER_H

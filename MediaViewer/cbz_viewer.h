#ifndef CBZ_VIEWER_H
#define CBZ_VIEWER_H

#include "image_viewer.h"
#include "../zip_reader.h"

class Cbz_Viewer : public Image_Viewer{
    zip_reader reader;
    int current_page = 0;
public:
    Cbz_Viewer(QLabel *label);

    void set_file(char *path) override;

    void read_page();

    void update_size() override;

    void next_page() override;

    void prev_page() override;

    QString get_info() override;

    inline bool multi_paged() override{return true;}
};

#endif // CBZ_VIEWER_H

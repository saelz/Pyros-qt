#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QPixmap>

#include "viewer.h"

class Image_Viewer : public Viewer
{
protected:
    QPixmap m_img;
    QSize scaled_size;
    double zoom_level;
    double zoom_increment = .25;

public:
    Image_Viewer(QLabel *label);
    ~Image_Viewer() override;

    void set_file(char *path) override;

    inline virtual QSize size(){return m_img.size();} ;
    virtual QSize current_size() override;

    virtual void set_size(QSize newsize);

    void resize(int width, int height, SCALE_TYPE scale) override;

    void update_size() override;

    void zoom_in() override;
    void zoom_out() override;

    QString get_info() override;

    inline bool resizable() override{return true;}
    inline bool scaleing() override{return true;}
};
#endif // IMAGE_VIEWER_H

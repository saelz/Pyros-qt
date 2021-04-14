#include <QLabel>

#include "image_viewer.h"

Image_Viewer::Image_Viewer(QLabel *label) : Viewer(label)
{
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    zoom_level = 1;
}

Image_Viewer::~Image_Viewer()
{
    m_label->setPixmap(QPixmap());
}

void Image_Viewer::set_file(char *path)
{
    zoom_level = 1;
    m_img = QPixmap(path);
}

QSize Image_Viewer::current_size()
{
    return scaled_size;
}

void Image_Viewer::set_size(QSize newsize){
    QPixmap scaledimg;

    scaled_size = newsize;

    scaledimg = m_img.scaled(newsize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    m_label->setPixmap(scaledimg);
};

void Image_Viewer::resize(int width, int height, Viewer::SCALE_TYPE scale) {
    if (scale_type != scale)
        zoom_level = 1;

    if (zoom_level != 1)
        return;

    Viewer::resize(width,height,scale);
}

void Image_Viewer::update_size()
{
    QSize newsize;

    newsize = size();
    switch (scale_type){
    case Viewer::HEIGHT:
        newsize.scale(newsize.width()*zoom_level,
                  boundry_height*zoom_level,
                  Qt::KeepAspectRatio);
        break;
    case Viewer::WIDTH:
        newsize.scale(boundry_width*zoom_level,
                  newsize.height()*zoom_level,
                  Qt::KeepAspectRatio);
        break;
    case Viewer::BOTH:
        newsize.scale(boundry_width*zoom_level,
                  boundry_height*zoom_level,
                  Qt::KeepAspectRatio);
        break;
    case Viewer::ORIGINAL:
        newsize.scale(newsize.width()*zoom_level,
                  newsize.height()*zoom_level,
                  Qt::KeepAspectRatio);
        break;
    }

    set_size(newsize);

}

void Image_Viewer::zoom_in()
{
    if ((zoom_level += zoom_increment) >= 3)
        zoom_level = 3;

    update_size();
}

void Image_Viewer::zoom_out()
{
    if ((zoom_level -= zoom_increment) <= 0)
        zoom_level = zoom_increment;

    update_size();
}

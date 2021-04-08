#include <QLocale>
#include <QLabel>
#include <QApplication>
#include "viewer.h"

Viewer::Viewer(QLabel *label) : m_label(label){};

Viewer::~Viewer(){};

void Viewer::resize(int width,int height,MediaViewer::SCALE_TYPE scale)
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

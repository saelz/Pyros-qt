#include <QLabel>

#include "unsupported_viewer.h"

Unsupported_Viewer::Unsupported_Viewer(QLabel *label) : Text_Viewer(label){};

void Unsupported_Viewer::set_file(char *path)
{
    Q_UNUSED(path);
    m_label->setText("Unsupported filetype");
}

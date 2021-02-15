#include <QLabel>

#include "cbz_viewer.h"

Cbz_Viewer::Cbz_Viewer(QLabel *label): Image_Viewer(label){}

void Cbz_Viewer::set_file(char *path)
{
    reader.read_file(path);

    if (reader.isValid)
        read_page();
    else
        m_label->setText("error reading cbz file");
}

void Cbz_Viewer::read_page()
{
    QByteArray data = reader.get_file_data(current_page);
    m_img.loadFromData(data);
    update_size();
}


void Cbz_Viewer::update_size()
{
    if (reader.isValid)
        Image_Viewer::update_size();
}

void Cbz_Viewer::next_page()
{
    if (!reader.isValid || current_page+1 >= reader.file_count())
        return;
    current_page++;
    read_page();
}

void Cbz_Viewer::prev_page()
{
    if (!reader.isValid || current_page <= 0)
        return;
    current_page--;
    read_page();
}

QString Cbz_Viewer::get_info()
{
    if (!reader.isValid)
        return "";

    return "Page:"+QString::number(current_page+1)+"/"+QString::number(reader.file_count())+
            " "+Image_Viewer::get_info();
}

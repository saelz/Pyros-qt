#include <QFile>
#include <QTextStream>
#include <QLabel>

#include "text_viewer.h"

Text_Viewer::Text_Viewer(QLabel *label): Viewer(label)
{
    QFont f("Arial", font_size);

    m_label->setFont(f);
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}

Text_Viewer::~Text_Viewer(){
    m_label->setText("");
}

void Text_Viewer::set_file(char *path)
{
    QFile file(path);
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        while (!stream.atEnd())
            line.append(stream.readLine()+"\n");

        m_label->setText(line);
    }
    file.close();

}

void Text_Viewer::zoom_in()
{
    font_size++;
    QFont f("Arial", font_size);
    m_label->setFont(f);
}

void Text_Viewer::zoom_out()
{
    if ((font_size--) == 0)
        font_size = 1;
    QFont f("Arial", font_size);
    m_label->setFont(f);
}

QString Text_Viewer::get_info()
{
    QString count = m_label->text();

    return "Word Count:"+QString::number(count.count(' ')+1);
}

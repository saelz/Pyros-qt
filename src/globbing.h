#ifndef GLOBBING_H
#define GLOBBING_H

#include <QString>

class Globbing
{
    static bool group_compare(QString glob, QChar c,int &glob_pos);
public:
    static bool glob_compare(QString glob, QString str,int glob_pos = 0,int str_pos = 0);
    static QByteArray escape_glob_characters(QString tag);
};

#endif // GLOBBING_H

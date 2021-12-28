#include "globbing.h"

#include <QString>


bool Globbing::group_compare(QString glob, QChar c,int &glob_pos)
{
    int start_pos = glob_pos;
    for (;glob_pos < glob.length(); glob_pos++){
        if (glob.at(glob_pos) == ']')
            return false;

        if (glob.at(glob_pos) == '-' &&
                glob_pos > start_pos+1 &&
                glob_pos < glob.length()-2 &&
                glob.at(glob_pos+1) != ']' &&
                glob.at(glob_pos+1) > glob.at(glob_pos-1)){

            if (glob.at(glob_pos-1) < c && c <= glob.at(glob_pos+1))
                goto found;
            glob_pos++;
        } else if (glob.at(glob_pos) == c){
            goto found;
        }

    }

    not_a_group:
    glob_pos = start_pos;
    if ('[' == c)
        return true;

    return false;

    found:
    for (;glob_pos < glob.length(); glob_pos++){
        if (glob.at(glob_pos) == ']')
            return true;
    }
    goto not_a_group;

}

bool Globbing::glob_compare(QString glob, QString str,int glob_pos, int str_pos)
{
    for (;str_pos < str.length(); str_pos++){
        if (glob.length()  <= glob_pos)
            return false;

        if (glob.at(glob_pos) == '*'){
            while(glob.length() > glob_pos && glob.at(glob_pos) == '*')
                glob_pos++;

            if (glob.length()  <= glob_pos)
                return true;

            while (!glob_compare(glob,str,glob_pos,str_pos)){
                str_pos++;
                if (str_pos >= str.length())
                    return false;
            }
        }
        if (glob.at(glob_pos) == '['){
            if (!group_compare(glob,str.at(str_pos),glob_pos))
                return false;
        } else if (glob.at(glob_pos) != str.at(str_pos) && glob.at(glob_pos) != '?'){
            return false;
        }

        glob_pos++;
    }

    for (;glob_pos < glob.length(); glob_pos++)
        if (glob.at(glob_pos) != '*')
            return false;

    return true;
}

QByteArray Globbing::escape_glob_characters(QString tag)
{
    QString escaped_tag;
    foreach (QChar c ,tag){
        if (c == '*' || c == '?' || c == '[' || c == ']'){
            escaped_tag.append('[');
            escaped_tag.append(c);
            escaped_tag.append(']');

        } else {
            escaped_tag.append(c);
        }
    }
    return escaped_tag.toUtf8();
}


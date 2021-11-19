#include <QString>

#include "playback_controller.h"

Playback_Controller::Playback_Controller(QObject *parent) : QObject(parent){};

QString Playback_Controller::milliToStr(unsigned long milli)
{
    unsigned long sec = milli/1000%60;
    unsigned long min = milli/(1000*60)%60;
    unsigned long hour =milli/(1000*60*60);

    if (show_milliseconds)
        return QString::asprintf("%02lu:%02lu.%03lu",min,sec,milli);
    else if (show_hours)
        return QString::asprintf("%lu:%02lu:%02lu",hour,min,sec);
    else
        return QString::asprintf("%02lu:%02lu",min,sec);

}

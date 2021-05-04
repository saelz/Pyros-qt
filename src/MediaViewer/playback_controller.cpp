#include <QTime>

#include "playback_controller.h"

Playback_Controller::Playback_Controller(QObject *parent) : QObject(parent){};

QString Playback_Controller::milliToStr(int milli)
{
    QTime duration(0,0,0);
    duration = duration.addMSecs(milli);

    if (show_milliseconds)
        return QString::asprintf("%02d:%02d.%03d",duration.minute(),duration.second(),duration.msec());
    else
        return QString::asprintf("%02d:%02d",duration.minute(),duration.second());

}

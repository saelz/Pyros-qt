#include <QTime>

#include "playback_controller.h"

Playback_Controller::Playback_Controller(QObject *parent) : QObject(parent){};

QString Playback_Controller::milliToStr(int milli)
{
    QTime duration(0,0,0);
    duration = duration.addMSecs(milli);

    if (show_milliseconds)
        return duration.toString("mm:ss.zzz");
    else if (show_hours)
        return duration.toString("hh:mm:ss");
    else
        return duration.toString("mm:ss");

}

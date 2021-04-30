#include <QTime>

#include "playback_controller.h"

Playback_Controller::Playback_Controller(QObject *parent) : QObject(parent){};

QString Playback_Controller::milliToStr(int milli)
{
    if (milli >= 1000){
        QTime duration(0,0,0);
        duration = duration.addMSecs(milli);
        return duration.toString();
    } else {
        return "00:00:00."+QString::number(milli);
    }

}

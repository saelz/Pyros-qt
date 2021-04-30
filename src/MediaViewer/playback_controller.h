#ifndef PLAYBACK_CONTROLLER_H
#define PLAYBACK_CONTROLLER_H

#include <QObject>

#include "viewer.h"

class Playback_Controller: public QObject{
    Q_OBJECT
protected:
    QString milliToStr(int milli);
public:
    Playback_Controller(QObject *parent);
    virtual QString duration() = 0;
    virtual QString position() = 0;

signals:
    void duration_changed(QString duration);
    void position_changed(QString position);
    void update_progress(int position,int max);
};

#endif // PLAYBACK_CONTROLLER_H

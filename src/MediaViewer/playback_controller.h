#ifndef PLAYBACK_CONTROLLER_H
#define PLAYBACK_CONTROLLER_H

#include <QObject>

#include "viewer.h"

class Playback_Controller: public QObject{
    Q_OBJECT
protected:
    QString milliToStr(int milli);

public:
    bool show_milliseconds = false;

    Playback_Controller(QObject *parent);
    virtual QString duration() = 0;
    virtual QString position() = 0;
    virtual bool pause_state() = 0;


public slots:
    virtual void fast_forward() = 0;
    virtual void rewind() = 0;
    virtual void pause() = 0;
    virtual void set_progress(double) = 0;

signals:
    void duration_changed(QString duration);
    void position_changed(QString position);
    void update_progress(int position,int max);
    void playback_state_changed(bool);
};

#endif // PLAYBACK_CONTROLLER_H

#ifndef OVERLAY_H
#define OVERLAY_H

#include "../viewer.h"

#include <QObject>
#include <QWidget>
#include <QTimer>

struct PyrosFile;
class MediaViewer;
class Overlay_Widget;

class Overlay : public QWidget
{
    Q_OBJECT

    bool show_zoom = false;
    bool show_page = false;

    Viewer **viewer;
    QTimer auto_hide_timer;
    PyrosFile *file = nullptr;
    Overlay_Widget *last_pressed_widget = nullptr;

public:

    struct Overlay_Bar{
        const int left_margin = 3;
        const int bottom_margin = 2;
        const int height = 20;
        const QBrush bg = QBrush(QColor(0,0,0,170));

        int unused_space = 0;
        bool active = true;

        QRect rect;
        QVector<Overlay_Widget*> widgets;
        void draw(Viewer *viewer,QPainter &p);
    };

    enum STATE{
        DISPLAYED,
        HIDDEN,
    };

    Overlay(Viewer **viewer,MediaViewer *parent);
    ~Overlay();
    bool auto_hide = true;
    bool locked = false;

    Overlay_Bar main_bar;
    Overlay_Bar playback_bar;

    std::array<Overlay_Bar*,2> overlay_bars{&main_bar,&playback_bar};

public slots:
    void set_visible();
    void set_hidden();
    void set_file(PyrosFile *file);

    bool mouseMoved(QMouseEvent *e);
    bool mouseClicked(QMouseEvent *e);
    bool mouseReleased(QMouseEvent *e);
    void toggle_lock();

private:
    STATE state = DISPLAYED;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

signals:
    void update_size_info(QString);
    void update_time_info(QString);
    void update_mime_info(QString);
    void update_file_info(QString);
    void update_playback_duration(QString);
    void update_playback_position(QString);
    void update_playback_progress(int,int);
    void update_playback_state(bool);
    void update_playback_volume(double);
    void update_playback_mute_state(bool);
    void update_playback_has_audio(bool);

    void fast_forward();
    void rewind();
    void pause();
    void change_progress(double);
    void change_volume(double);
    void change_muted(bool);

};

#endif // OVERLAY_H

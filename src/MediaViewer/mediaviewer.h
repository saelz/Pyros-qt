#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QTimer>

#include "viewer.h"

class QStackedWidget;
class QLabel;
class QScrollArea;
class QScrollEvent;

class mpv_widget;
class Viewer;
struct PyrosFile;
class Overlay;


class MediaViewer : public QWidget
{
    Q_OBJECT
    enum StackedWidgetLayers{
        VIDEO_LAYER,
        LABEL_LAYER,
    };

    QStackedWidget *stacked_widget;
    QScrollArea *scroll_area;
    QLabel *label;
    mpv_widget *video_player;
    Viewer *viewer = nullptr;
    QPoint last_mouse_pos;

    QVector<PyrosFile*> files;
    int file_position = -1;

    bool mouse_clicked = false;

public:
    explicit MediaViewer(QWidget *parent = nullptr);
    ~MediaViewer();
    static bool is_apng(char*);


    Overlay *overlay;
    bool files_deletable = false;

    bool slideshows_enabled           = false;
    bool slideshow_active             = false;
    int  slideshow_wait_time          = 1000;
    bool slideshow_watch_entire_video = false;
    bool slideshow_random_order       = false;
    bool slideshow_loop               = false;

    QTimer slide_timer;

    void bind_keys(QWidget *widget,bool files_deletable = false);
    inline void set_slideshows_enabled(bool slideshows_enabled){this->slideshows_enabled = slideshows_enabled;};

public slots:
    void set_files(QVector<PyrosFile*> files,int inital_pos = 0);
    void next_file();
    void prev_file();
    void random_file();
    void set_current_file(int position);
    void hide_files(QVector<QByteArray> hashes);

    void open_slideshow_conf();
    void toggle_slideshow();
    void next_slide();


    void next_page();
    void prev_page();
    void delete_file();

    void zoom_out();
    void zoom_in();

    void set_scale(int);

    void set_focus();

    void toggle_fullscreen();

    inline const PyrosFile *file_at(int i){return files.at(i);};

private:
    Viewer::SCALE_TYPE scale_type = Viewer::SCALE_TYPE::BOTH;

    void set_file();
    bool is_dragable();
    void resizeEvent(QResizeEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void showEvent(QShowEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *) override;

    void update_scale();

signals:
    void lock_overlay();
    void file_removed_at(int);
    void file_changed(PyrosFile *file);
    void position_changed(int);
    void update_file_count(QString);
    void file_deleted(QVector<QByteArray>);
    void slideshow_ended();
    void slideshow_started();
};

#endif // MEDIAVIEWER_H

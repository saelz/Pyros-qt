#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>

#include "viewer.h"

class QStackedWidget;
class QLabel;
class QScrollArea;

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

    bool mouse_clicked = false;

public:
    explicit MediaViewer(QWidget *parent = nullptr);
    ~MediaViewer();


    Overlay *overlay;

    void bind_keys(QWidget *widget);

public slots:
    void set_file(PyrosFile* file);


    void next_page();
    void prev_page();

    void zoom_out();
    void zoom_in();

    void set_scale(int);

    void set_focus();


private:
    Viewer::SCALE_TYPE scale_type = Viewer::SCALE_TYPE::BOTH;

    bool is_dragable();
    void resizeEvent(QResizeEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void showEvent(QShowEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void update_scale();
signals:
    void lock_overlay();
};

#endif // MEDIAVIEWER_H

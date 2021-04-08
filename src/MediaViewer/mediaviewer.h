#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QTimer>

struct PyrosFile;

class QStackedWidget;
class QLabel;
class QScrollArea;

class mpv_widget;
class Viewer;

class Overlay : public QWidget
{
    Q_OBJECT

   Viewer **viewer;
   QTimer auto_hide_timer;
   PyrosFile *file = nullptr;

public:
    enum STATE{
        DISPLAYED,
        HIDDEN,
    };

    Overlay(Viewer **viewer,QWidget *parent = nullptr);
    bool auto_hide = true;

public slots:
    void set_visible();
    void set_hidden();
    void set_file(PyrosFile *file){this->file = file;};

private:
   STATE state = DISPLAYED;
   void paintEvent(QPaintEvent *) override;


};

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
    Overlay *overlay;
    QPoint last_mouse_pos;

public:
    explicit MediaViewer(QWidget *parent = nullptr);
    ~MediaViewer();

    enum SCALE_TYPE{
        HEIGHT,
        WIDTH,
        BOTH,
        ORIGINAL,
    };

public slots:
    void set_file(PyrosFile* file);

    bool is_resizable();
    bool is_multipaged();

    void next_page();
    void prev_page();

    void zoom_out();
    void zoom_in();

    void set_scale(SCALE_TYPE scale);

    void set_focus();


private:
    SCALE_TYPE scale_type = SCALE_TYPE::BOTH;

    bool is_dragable();
    void resizeEvent(QResizeEvent *) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
    void showEvent(QShowEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void update_scale();
};

#endif // MEDIAVIEWER_H

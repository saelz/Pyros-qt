#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QTimer>

#include "viewer.h"

struct PyrosFile;

class QStackedWidget;
class QLabel;
class QScrollArea;

class mpv_widget;
class Viewer;
class MediaViewer;
class Overlay;

class Overlay_Widget
{
public:
    QRect rect = QRect(0,0,0,0);
    virtual int requested_width(QPainter &p) = 0;
    virtual int draw(QPainter &p,int x,int y) = 0;
    virtual void check_hover(QMouseEvent *e);
    virtual void clicked(){};
    QString tooltip;
};

class Overlay_Text : public QObject,public Overlay_Widget
{
    Q_OBJECT
    QString text;
public:
    Overlay_Text(QString tooltip,Overlay *parent);
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
public slots:
    void set_text(QString text);

signals:
    void request_redraw(void);

};

class Overlay_Button :public QObject,public Overlay_Widget
{
    Q_OBJECT
public:
    Overlay_Button(QByteArray icon_path,bool *active_ptr,QString tooltip,Overlay *parent);
    QImage icon;
    int width = 16;
    int height = 16;
    bool highlighed = false;
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
    void check_hover(QMouseEvent *e) override;


    bool *active;
signals:
    void clicked() override;
    void request_redraw(void);

};

class Overlay_Spacer: public Overlay_Widget
{
public:
    Overlay_Spacer(int *available_space);
    inline int requested_width(QPainter &) override{return 0;};
    int draw(QPainter &p,int x, int y) override;
    int *unused_space;
};

class Overlay_Combo_Box : public Overlay_Button
{
    Q_OBJECT

    bool display_dropdown = false;
    QVector<QRect> dropdownrect;
public:
    Overlay_Combo_Box(bool *active_ptr,QString tooltip,Overlay *parent);

    struct Combo_Entry{
        QString name;
        int value;
    };

    QVector<Combo_Entry> entries;
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
    void check_hover(QMouseEvent *e) override;


    int selected_entry = 0;
    int highlighted_entry = -1;

private slots:
    void toggle_drop_down();

signals:
    void entry_changed(int);

};


class Overlay : public QWidget
{
    Q_OBJECT


    const int left_margin = 3;
    const int bottom_margin = 2;
    const int height = 20;

    bool show_zoom = false;
    bool show_page = false;
    int unused_space = 0;

    Viewer **viewer;
    QTimer auto_hide_timer;
    PyrosFile *file = nullptr;
    Overlay_Widget *last_pressed_widget = nullptr;

public:
    enum STATE{
        DISPLAYED,
        HIDDEN,
    };

    Overlay(Viewer **viewer,MediaViewer *parent);
    bool auto_hide = true;

    QVector<Overlay_Widget*> overlay_widgets;

public slots:
    void set_visible();
    void set_hidden();
    void set_file(PyrosFile *file);

    bool mouseMoved(QMouseEvent *e);
    bool mouseClicked(QMouseEvent *e);
    bool mouseReleased(QMouseEvent *e);

private:
    STATE state = DISPLAYED;
    void paintEvent(QPaintEvent *) override;

signals:
    void update_size_info(QString);
    void update_time_info(QString);
    void update_mime_info(QString);
    void update_file_info(QString);

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
    QPoint last_mouse_pos;

    bool mouse_clicked = false;

public:
    explicit MediaViewer(QWidget *parent = nullptr);
    ~MediaViewer();


    Overlay *overlay;

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
};

#endif // MEDIAVIEWER_H

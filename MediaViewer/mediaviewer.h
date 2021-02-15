#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>

struct PyrosFile;

class QStackedWidget;
class QLabel;
class QScrollArea;

class mpv_widget;
class Viewer;

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

signals:
    void info_updated(QString);

private:
    SCALE_TYPE scale_type = SCALE_TYPE::BOTH;
    bool eventFilter(QObject *obj,QEvent *event) override;
    void update_scale();

};

#endif // MEDIAVIEWER_H

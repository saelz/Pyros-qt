#ifndef MPV_WIDGET_H
#define MPV_WIDGET_H

#include <QWidget>

#include <mpv/client.h>

class mpv_widget : public QWidget
{
    Q_OBJECT
public:
    explicit mpv_widget(QWidget *parent = nullptr);
    ~mpv_widget();

    void set_file(char *path);
    void stop();
private:
    mpv_handle *mpv;
    void handle_mpv_event(mpv_event *event);

signals:
    void mpv_events();

private slots:
    void on_mpv_events();

};

#endif // MPV_WIDGET_H

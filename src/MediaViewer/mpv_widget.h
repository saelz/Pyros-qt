#ifndef MPV_WIDGET_H
#define MPV_WIDGET_H

#include <QOpenGLWidget>

struct mpv_handle;
struct mpv_event;
struct mpv_render_context;
struct mpv_event_property;

class mpv_widget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit mpv_widget(QWidget *parent = nullptr);
    ~mpv_widget();

    void init();
    bool is_paused();

public slots:
    void set_file(char *path);
    void stop();
    void toggle_playback();
    void rewind();
    void fast_forward();
    void set_progress(double);
    void set_volume(double);

private:
    bool stop_playback_updates = false;
    bool initalized = false;
    mpv_handle *mpv = nullptr;
    mpv_render_context *mpv_gl = nullptr;

    bool check_prop(mpv_event_property*prop,int format,const char *name);

    void handle_mpv_event(mpv_event *event);
    static void wakeup(void *ctx);
    static void *get_proc_address(void *, const char *name);
    static void on_update(void *ctx);

    void initializeGL() override;
    void paintGL() override;


signals:
    void mpv_events();
    void mpv_update();
    void duration_changed(double);
    void position_changed(double);
    void volume_changed(double);
    void playback_state(bool);

private slots:
    void mpv_event_occured();
    void invoke_update();

};

#endif // MPV_WIDGET_H

#include <QAction>
#include <QOpenGLWidget>
#include <QOpenGLContext>

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include "mpv_widget.h"

void mpv_widget::wakeup(void *ctx)
{
    mpv_widget *mpv_wid = (mpv_widget *)ctx;
    emit mpv_wid->mpv_events();
}

mpv_widget::mpv_widget(QWidget *parent) : QOpenGLWidget(parent)
{
    QAction *pause = new QAction("pause",this);
    QAction *seek_right = new QAction("pause",this);
    QAction *seek_left = new QAction("pause",this);

    pause->setShortcut(QKeySequence("Space"));
    seek_right->setShortcut(QKeySequence("Right"));
    seek_left->setShortcut(QKeySequence("Left"));



    addAction(pause);
    addAction(seek_right);
    addAction(seek_left);

    connect(pause,&QAction::triggered,this,&mpv_widget::toggle_playback);
    connect(seek_right,&QAction::triggered,this,&mpv_widget::quick_fast_forward);
    connect(seek_left,&QAction::triggered,this,&mpv_widget::quick_rewind);
}

void mpv_widget::init()
{
    if (initalized)
        return;

    mpv = mpv_create();
    if (!mpv)
        return;

    mpv_set_option_string(mpv, "loop", "yes");

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    connect(this, &mpv_widget::mpv_events, this, &mpv_widget::mpv_event_occured,
            Qt::QueuedConnection);

    connect(this, &mpv_widget::mpv_update, this, &mpv_widget::invoke_update,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);


    if (mpv_initialize(mpv) < 0)
        return;

    initalized = true;

}

mpv_widget::~mpv_widget()
{
    if (initalized)
        makeCurrent();

    if (mpv_gl != nullptr)
        mpv_render_context_free(mpv_gl);

    if (mpv != nullptr)
        mpv_terminate_destroy(mpv);
}

void mpv_widget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_SHUTDOWN:
        mpv_terminate_destroy(mpv);
        mpv = NULL;
        initalized = false;
        break;

    case MPV_EVENT_PROPERTY_CHANGE:{
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                qDebug("TIM:%lf",time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                qDebug("DUR:%lf",time);
            }
        }
        break;
    }

    default: ;
    }
}

void mpv_widget::mpv_event_occured()
{
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

void mpv_widget::set_file(char *path)
{
    if (!initalized)
        init();

    if (mpv) {
        const char *args[] = {"loadfile", path, NULL};
        mpv_command_async(mpv, 0, args);
    }
}
void mpv_widget::stop()
{
    if (mpv) {
        const char *args[] = {"stop", NULL};
        mpv_command_async(mpv, 0, args);
    }
}

void mpv_widget::toggle_playback()
{
    if (mpv) {
        const char *args[] = {"cycle","pause", NULL};
        mpv_command_async(mpv, 0, args);
    }
}

void mpv_widget::quick_rewind()
{
    if (mpv) {
        const char *args[] = {"seek","-3", NULL};
        mpv_command_async(mpv, 0, args);
    }

}
void mpv_widget::quick_fast_forward()
{
    if (mpv) {
        const char *args[] = {"seek","3", NULL};
        mpv_command_async(mpv, 0, args);
    }

}

void *mpv_widget::get_proc_address(void *,const char *name)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx == nullptr)
        return nullptr;

    return (void*)ctx->getProcAddress(name);

}

void mpv_widget::initializeGL()
{
    mpv_opengl_init_params gl_init_params = {get_proc_address,nullptr,nullptr};
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if (!initalized)
        init();

    if (mpv_render_context_create(&mpv_gl,mpv,params) < 0){
        return;
    }

    mpv_render_context_set_update_callback(mpv_gl,mpv_widget::on_update,(void*)this);
}

void mpv_widget::paintGL()
{
    mpv_opengl_fbo mpfbo = {static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    int flip_y = {1};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    mpv_render_context_render(mpv_gl, params);
}

void mpv_widget::invoke_update()
{
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void mpv_widget::on_update(void *ctx)
{

    mpv_widget *mpv_wid = (mpv_widget *)ctx;
    emit mpv_wid->mpv_update();
}

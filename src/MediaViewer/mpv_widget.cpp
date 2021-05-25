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

mpv_widget::mpv_widget(QWidget *parent) : QOpenGLWidget(parent){}

void mpv_widget::init()
{
    if (initalized)
        return;

    mpv = mpv_create();
    if (!mpv)
        return;

    mpv_set_option_string(mpv, "loop", "yes");

    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_STRING);
    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-remaining", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_STRING);

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


bool mpv_widget::check_prop(mpv_event_property*prop,int format,const char *name)
{
    if (strcmp(prop->name, name) == 0)
        if (prop->format == format)
            return true;

    return false;
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
        if (check_prop(prop,MPV_FORMAT_DOUBLE,"time-pos")) {
            if (!stop_playback_updates)
                emit position_changed(*(double *)prop->data);

        } else if (check_prop(prop,MPV_FORMAT_DOUBLE, "duration")) {
            emit duration_changed(*(double *)prop->data);

        } else if (check_prop(prop,MPV_FORMAT_DOUBLE, "volume")) {
            emit volume_changed(*(double *)prop->data);
        } else if (check_prop(prop,MPV_FORMAT_DOUBLE, "time-remaining")) {
            emit remaining_changed(*(double *)prop->data);

        } else if (check_prop(prop,MPV_FORMAT_STRING, "pause")) {
            if (**(char**)prop->data == 'n')
                emit playback_state(true);
            else
                emit playback_state(false);
        }
        break;
    }

    case MPV_EVENT_SEEK:
        stop_playback_updates = false;
        break;

    case MPV_EVENT_FILE_LOADED:
        emit has_audio(check_if_has_audio());
        invoke_update();
        break;

    default:
        break;
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

bool mpv_widget::is_paused()
{
    char *pause_state = nullptr;
    bool state;

    mpv_get_property(mpv, "pause", MPV_FORMAT_STRING,&pause_state);
    if (pause_state == nullptr || *pause_state == 'n')
        state = true;
    else
        state = false;

    mpv_free(pause_state);
    return state;
}

void mpv_widget::set_file(char *path)
{
    if (!initalized)
        init();

    if (mpv) {
        const char *args[] = {"loadfile", path, NULL};

        stop_playback_updates = false;
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

void mpv_widget::rewind()
{
    if (mpv) {
        const char *args[] = {"seek","-1", NULL};
        stop_playback_updates = true;
        mpv_command_async(mpv, 0, args);
    }

}
void mpv_widget::fast_forward()
{
    if (mpv) {
        const char *args[] = {"seek","1", NULL};
        stop_playback_updates = true;
        mpv_command_async(mpv, 0, args);
    }

}

void mpv_widget::set_progress(double progress)
{
    if (mpv) {
        const char *args[] = {"seek",QString::number(progress*100).toUtf8(),"absolute-percent+exact", NULL};
        stop_playback_updates = true;
        mpv_command_async(mpv, 0, args);
    }

}

void mpv_widget::set_volume(double volume)
{
    if (mpv) {
        const char *args[] = {"set","volume",QString::number(volume*100).toUtf8(), NULL};
        stop_playback_updates = true;
        mpv_command_async(mpv, 0, args);
    }

}

bool mpv_widget::check_if_has_audio()
{
    char *acodec = nullptr;
    bool state = true;

    mpv_get_property(mpv, "audio-codec", MPV_FORMAT_STRING,&acodec);
    if (acodec == nullptr)
        state = false;

    mpv_free(acodec);
    return state;

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

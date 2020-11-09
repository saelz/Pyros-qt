#include "mpv_widget.h"

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    mpv_widget *player = (mpv_widget *)ctx;
    emit player->mpv_events();
}

mpv_widget::mpv_widget(QWidget *parent) : QWidget(parent)
{
    mpv = mpv_create();
    if (!mpv)
        return;

    // Create a video child window. Force Qt to create a native window, and
    // pass the window ID to the mpv wid option. Works on: X11, win32, Cocoa

    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
    // If you have a HWND, use: int64_t wid = (intptr_t)hwnd;
    int64_t wid = winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(mpv, "input-default-bindings", "yes");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(mpv, "input-vo-keyboard", "yes");
    mpv_set_option_string(mpv, "loop", "yes");
    mpv_set_option_string(mpv, "osc","yes");
    mpv_set_option_string(mpv, "force-window","yes");

    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "chapter-list", MPV_FORMAT_NODE);

    // Request log messages with level "info" or higher.
    // They are received as MPV_EVENT_LOG_MESSAGE.
    //mpv_request_log_messages(mpv, "info");

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &mpv_widget::mpv_events, this, &mpv_widget::on_mpv_events,
            Qt::QueuedConnection);
    mpv_set_wakeup_callback(mpv, wakeup, this);

    if (mpv_initialize(mpv) < 0)
        return;//lol
}

mpv_widget::~mpv_widget()
{
    if (mpv)
        mpv_terminate_destroy(mpv);
}

void mpv_widget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            /*if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                std::stringstream ss;
                ss << "At: " << time;
                statusBar()->showMessage(QString::fromStdString(ss.str()));
            } else if (prop->format == MPV_FORMAT_NONE) {
                // The property is unavailable, which probably means playback
                // was stopped.
                statusBar()->showMessage("");
            }*/
        } else if (strcmp(prop->name, "chapter-list") == 0 ||
                   strcmp(prop->name, "track-list") == 0)
        {
            // Dump the properties as JSON for demo purposes.
/*#if QT_VERSION >= 0x050000
            if (prop->format == MPV_FORMAT_NODE) {
                QVariant v = mpv::qt::node_to_variant((mpv_node *)prop->data);
                // Abuse JSON support for easily printing the mpv_node contents.
                QJsonDocument d = QJsonDocument::fromVariant(v);
                append_log("Change property " + QString(prop->name) + ":\n");
                append_log(d.toJson().data());
            }
#endif*/
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        // Retrieve the new video size.
        int64_t w, h;
        if (mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            // Note that the MPV_EVENT_VIDEO_RECONFIG event doesn't necessarily
            // imply a resize, and you should check yourself if the video
            // dimensions really changed.
            // mpv itself will scale/letter box the video to the container size
            // if the video doesn't fit.
            /*std::stringstream ss;
            ss << "Reconfig: " << w << " " << h;
            statusBar()->showMessage(QString::fromStdString(ss.str()));*/
        }
        break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
        //struct mpv_event_log_message *msg = (struct mpv_event_log_message *)event->data;
        /*std::stringstream ss;
        ss << "[" << msg->prefix << "] " << msg->level << ": " << msg->text;
        append_log(QString::fromStdString(ss.str()));*/
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

void mpv_widget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

void mpv_widget::set_file(char *path)
{
    if (mpv) {
        //const qbytearray c_filename = filename.toutf8();
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

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <QObject>
struct AVFormatContext;
struct AVCodec;
struct AVFrame;
struct AVPacket;
struct AVCodecContext;
struct AVCodecParameters;
struct SwsContext;

class VideoCodec{
    class codec_stream{
    public:
        codec_stream();
        codec_stream(AVFormatContext *format_ctx,int codec_type);
        void set_codec(AVFormatContext *format_ctx,int codec_type);

        long stream_pos = -1;
        const AVCodec *codec = nullptr;
        AVCodecParameters *codec_para = nullptr;
        AVCodecContext *codec_ctx = nullptr;
    };

    class frame{
    public:
        frame();
        ~frame();
        QImage read_next_frame(codec_stream video_stream);
        AVFrame *avframe = nullptr;
        AVPacket *packet = nullptr;
        SwsContext *swsCtx = nullptr;
    };

public:
    VideoCodec();
    ~VideoCodec();
    VideoCodec(QByteArray path);
    bool set_file(QByteArray path);
    QPixmap get_first_frame();

    bool is_valid = false;
    AVFormatContext *format_ctx = nullptr;
    codec_stream video_stream;

};

class Video_thumbnailer : public QObject
{
    Q_OBJECT
public:
   Video_thumbnailer(QObject *parent = nullptr);

   static QPixmap generate_thumbnail(QByteArray path);
signals:

};


#endif // VIDEO_THUMBNAILER_H

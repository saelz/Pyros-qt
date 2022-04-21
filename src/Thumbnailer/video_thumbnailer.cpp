#include "video_thumbnailer.h"

#include <QPixmap>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

VideoCodec::VideoCodec(){}

VideoCodec::~VideoCodec(){
    avformat_close_input(&format_ctx);
    avcodec_free_context(&video_stream.codec_ctx);
}

VideoCodec::VideoCodec(QByteArray path)
{
    set_file(path);
}

VideoCodec::codec_stream::codec_stream(AVFormatContext *format_ctx,int codec_type)
{
    set_codec(format_ctx,codec_type);
}

VideoCodec::codec_stream::codec_stream(){}

void VideoCodec::codec_stream::set_codec(AVFormatContext *format_ctx,int codec_type)
{

    for	(unsigned i = 0; i < format_ctx->nb_streams; i++){
        codec_para = format_ctx->streams[i]->codecpar;
        codec = avcodec_find_decoder(codec_para->codec_id);

        if (codec_para->codec_type == codec_type){
            stream_pos = i;
            codec_ctx = avcodec_alloc_context3(codec);
            if (codec_ctx == nullptr)
                return;

            if (avcodec_parameters_to_context(codec_ctx,codec_para) < 0)
                return;

            if (avcodec_open2(codec_ctx,codec,NULL) < 0)
                return;

        }
    }
}


bool VideoCodec::set_file(QByteArray path)
{
    is_valid = false;

    format_ctx = avformat_alloc_context();

    if (format_ctx == nullptr)
        return false;

    if (avformat_open_input(&format_ctx,path,nullptr,nullptr) < 0)
        return false;

    if (avformat_find_stream_info(format_ctx,NULL) < 0)
        return false;


    video_stream.set_codec(format_ctx,AVMEDIA_TYPE_VIDEO);

    if (video_stream.stream_pos == -1)
        return false;

    is_valid = true;

    return true;
};

VideoCodec::frame::frame()
{
    avframe = av_frame_alloc();
    packet = av_packet_alloc();

    if (packet == nullptr || avframe == nullptr){
        return;
    }
}
VideoCodec::frame::~frame()
{
    av_packet_free(&packet);
    av_frame_free(&avframe);
    sws_freeContext(swsCtx);
}

QImage VideoCodec::frame::read_next_frame(codec_stream video_stream)
{
    AVPixelFormat fmt = AV_PIX_FMT_RGB24;
    int alignment = 24;
    uint8_t *rgb_data[4];
    int rgb_linesize[4];

    avcodec_send_packet(video_stream.codec_ctx,packet);
    if (avcodec_receive_frame(video_stream.codec_ctx,avframe) == 0){
        QImage img = QImage(avframe->width,avframe->height, QImage::Format_RGB32);
        av_image_alloc(rgb_data,rgb_linesize,avframe->width,avframe->height,fmt,alignment);

        if (avframe->format == -1)
            avframe->format = video_stream.codec_ctx->pix_fmt;
        swsCtx = sws_getCachedContext(swsCtx,avframe->width,avframe->height,
                      (AVPixelFormat)avframe->format,
                      avframe->width,avframe->height,
                      fmt,SWS_BILINEAR,
                      NULL,NULL,NULL);

        sws_scale(swsCtx,avframe->data,avframe->linesize,0,avframe->height,
              rgb_data,rgb_linesize);

        uint8_t *src = (uint8_t *)(rgb_data[0]);
        for (int y = 0; y < avframe->height; y++){
            QRgb *scanLine = (QRgb *) img.scanLine(y);
            for (int x = 0; x < avframe->width; x++)
                scanLine[x] = qRgb(src[3*x], src[3*x+1], src[3*x+2]);

            src += rgb_linesize[0];
        }


        av_freep(&rgb_data[0]);
        return img;
    }

    return QImage();
}

QPixmap VideoCodec::get_first_frame()
{
    frame vid_frame;

    while (av_read_frame(format_ctx,vid_frame.packet) >= 0){
        if (vid_frame.packet->stream_index == video_stream.stream_pos){
            QImage img = vid_frame.read_next_frame(video_stream);
            if (!img.isNull()){
                av_packet_unref(vid_frame.packet);
                return QPixmap::fromImage(img);
            }
        }
        av_packet_unref(vid_frame.packet);
    }

    return QPixmap();
}


Video_thumbnailer::Video_thumbnailer(QObject *parent)
    : QObject{parent}
{}

QPixmap Video_thumbnailer::generate_thumbnail(QByteArray path)
{
    VideoCodec vc(path);
    if (!vc.is_valid)
        return QPixmap();

    QPixmap pix = vc.get_first_frame();
    return pix;
}

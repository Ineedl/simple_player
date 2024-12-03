//
// Created by cjh on 2024/10/29.
//

#include "ReceiveThread.h"
#include <QDebug>
#include <QByteArray>
#include "libavutil/samplefmt.h"
#include "libavutil/pixfmt.h"



int ReceiveThread::init() {

    isRun= true;
    fmt_ctx   = NULL;
    codec_ctx = NULL;
    pPacket        = NULL;

    QString filename  = "rtsp://192.168.15.230:554/H264?ch=1&subtype=0&proto=Onvif";
//    QString filename  = "2.mp4";
    AVDictionary *options = NULL;

    // 设置使用 TCP
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    //打开输入文件
    if (avformat_open_input(&fmt_ctx, filename.toUtf8().data(), NULL, &options) < 0) {
        qDebug()<<"Could not open source file "<< filename;
        return -1;
    }
    if(avformat_find_stream_info(fmt_ctx,NULL) < 0)
    {
        qDebug()<<"get stream fail"<<endl;
        return -1;
    }
    //查找视频索引
    video_index = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);

    if(video_index < 0)
    {
        qDebug()<<"not find video index"<<endl;
        return -1;
    }


    audio_index = av_find_best_stream(fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    if(audio_index < 0)
    {
        qDebug()<<"not find audio index"<<endl;
        return -1;
    }


    //打印输入文件相关信息
    av_dump_format(fmt_ctx, -1,filename.toUtf8().data(),0);

    //查找解码器
    const AVCodec *vCodec = NULL;
    vCodec = avcodec_find_decoder(fmt_ctx->streams[video_index]->codecpar->codec_id);
    if(!vCodec)
    {
        qDebug()<<"find video decoder fail"<<endl;
        return -1;
    }

    //初始化解码器上下文
    codec_ctx = avcodec_alloc_context3(vCodec);
    if(!codec_ctx)
    {
        qDebug()<<"init codec_ctx fail"<<endl;
        return -1;
    }


    //查找解码器
    const AVCodec *audio_vCodec = NULL;
    audio_vCodec = avcodec_find_decoder(fmt_ctx->streams[audio_index]->codecpar->codec_id);
    if(!audio_vCodec)
    {
        qDebug()<<"find audio decoder fail"<<endl;
        return -1;
    }

    //初始化解码器上下文
    audio_codec_ctx = avcodec_alloc_context3(audio_vCodec);
    if(!audio_codec_ctx)
    {
        qDebug()<<"init audio codec_ctx fail"<<endl;
        return -1;
    }

    qDebug()<<"codec: "<<vCodec->long_name;
    qDebug()<<"codec bit rate: "<<codec_ctx->bit_rate;
    qDebug()<<"fmt_ctx bit rate: "<<fmt_ctx->bit_rate/1000;

    // 获取时间基准
    time_base = fmt_ctx->streams[video_index]->time_base;
    audio_time_base = fmt_ctx->streams[audio_index]->time_base;
    int ret = avcodec_parameters_to_context(codec_ctx,fmt_ctx->streams[video_index]->codecpar);
    if(ret < 0)
    {
        qDebug()<<"avcodec_parameters_to_contex fail"<<endl;
        return -1;
    }

    ret = avcodec_parameters_to_context(audio_codec_ctx,fmt_ctx->streams[audio_index]->codecpar);
    if(ret < 0)
    {
        qDebug()<<"audio avcodec_parameters_to_contex fail"<<endl;
        return -1;
    }

    //打开解码器
    ret = avcodec_open2(codec_ctx,vCodec,NULL);
    if(ret != 0)
    {
        qDebug()<<"video codec open fail"<<endl;
        return -1;
    }

    ret = avcodec_open2(audio_codec_ctx,audio_vCodec,NULL);
    if(ret != 0)
    {
        qDebug()<<"audio codec open fail"<<endl;
        return -1;
    }

    pPacket  = av_packet_alloc();

    qDebug()<<"width="<< codec_ctx->width<<" height="<< codec_ctx->height<<endl;
}

void ReceiveThread::run() {
    while(isRun)
    {
        int ret = av_read_frame(fmt_ctx, pPacket);
        if (ret != 0)
        {
            av_packet_unref(pPacket);
            break;
        }
        AVCodecContext* now_codec_ctx=codec_ctx;
        if(pPacket->stream_index == video_index){
            now_codec_ctx=codec_ctx;
        }else if(pPacket->stream_index == audio_index){
            now_codec_ctx=audio_codec_ctx;
        }else{
            continue;
        }
        if(pPacket->size==0||pPacket->data== nullptr){
            av_packet_unref(pPacket);
            continue;
        }

        // 发送待解码包
        int success = avcodec_send_packet(now_codec_ctx, pPacket);
        if (success < 0)
        {
            //清理AVPacket中的数据并且重新初始化
            av_packet_unref(pPacket);
            continue;
        }
        //接收解码数据
        while (success >= 0)
        {
            AVFrame *decoded_frame;
            decoded_frame = av_frame_alloc();
            if (!decoded_frame) {
                qDebug()<< "create decoded frame err";
                continue;
            }
            success = avcodec_receive_frame(now_codec_ctx, decoded_frame);
            if (success == AVERROR_EOF)
            {
                break;
            }
            else if (success == AVERROR(EAGAIN))
            {
                success = 0;
                break;
            }
            else if (success < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Error decoding frame\n");
                //清理AVFrame中的数据并且重新初始化
                av_frame_unref(decoded_frame);
                break;
            }
            if(now_codec_ctx==audio_codec_ctx){
                decoded_frame->time_base = audio_time_base;
                emit signalAudioSendData(decoded_frame);
//                qDebug()<<"audio format:"<<av_get_sample_fmt_name(AVSampleFormat(decoded_frame->format));
            }else{
                decoded_frame->time_base = time_base;
                emit signalVideoSendData(decoded_frame);
//                qDebug()<<"video format:"<<av_get_pix_fmt_name (AVPixelFormat(decoded_frame->format));
            }


        }
        av_packet_unref(pPacket);
    }



}

ReceiveThread::ReceiveThread(){
    init();
}

ReceiveThread::~ReceiveThread(){
    while (isRun){}
    //释放
    avcodec_free_context(&codec_ctx);
    avcodec_free_context(&audio_codec_ctx);
    av_packet_free(&pPacket);
    avformat_close_input(&fmt_ctx);
}

void ReceiveThread::stop() {
    isRun = false;
}
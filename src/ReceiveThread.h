//
// Created by cjh on 2024/10/29.
//

#ifndef PALYER_RECEIVETHREAD_H
#define PALYER_RECEIVETHREAD_H


#include <QThread>
#include <QTimer>
#if defined __cplusplus
#define __STDC_CONSTANT_MACROS  //common.h中的错误
#define __STDC_FORMAT_MACROS    //timestamp.h中的错误
#endif

extern "C"
{
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

class ReceiveThread : public QThread{
Q_OBJECT
signals:
    void signalVideoSendData(AVFrame* decode);
    void signalAudioSendData(AVFrame* decode);
public:
    ReceiveThread();
    ~ReceiveThread();
    void stop();

private:
    int init();
    void run() override;


private:
    AVFormatContext *fmt_ctx;
    AVCodecContext  *codec_ctx;
    AVCodecContext  *audio_codec_ctx;
    AVPacket   *pPacket;

    AVRational time_base;
    AVRational audio_time_base;
    int video_index;
    int audio_index;
    bool isRun;

};


#endif //PALYER_RECEIVETHREAD_H

//
// Created by cjh on 2024/10/30.
//

#ifndef PALYER_SENDTHREAD_H
#define PALYER_SENDTHREAD_H

#include <QThread>
#include <QTimer>
#include <QQueue>
#include <QMutex>

struct PlayData{
    QByteArray data;
    int width;
    int height;
    int64_t pts;
    int type;
    int num_samples;
};

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

class VideoDisposeThread : public QThread{
Q_OBJECT

signals:
    void signalPlay(PlayData data);
    void signalVideoPTSModify(int64_t pts);

public slots:
    void slotReceiveVideoData(AVFrame* decode);

public:
    VideoDisposeThread();
    ~VideoDisposeThread();
    void stop();
private:
    int init();
    void run() override;

    AVFrame *rgb_frame;

    int64_t pts;
    QQueue<PlayData> queue;
    QMutex mutex;
    struct SwsContext *sws_ctx;
    bool isRun;

};


#endif //PALYER_SENDTHREAD_H

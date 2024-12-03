//
// Created by cjh on 2024/10/31.
//

#ifndef PALYER_AUDIODISPOSETHREAD_H
#define PALYER_AUDIODISPOSETHREAD_H

#if defined __cplusplus
#define __STDC_CONSTANT_MACROS  //common.h中的错误
#define __STDC_FORMAT_MACROS    //timestamp.h中的错误
#endif

#include <QThread>
#include "VideoDisposeThread.h"

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

class AudioDisposeThread : public QThread{
    Q_OBJECT
            signals:
            void signalAudioPlay(PlayData data);

public slots:
            void slotReceiveAudioData(AVFrame* decode);
            void slotSetPTS(int64_t);

public:
    AudioDisposeThread();
    ~AudioDisposeThread();
    void stop();
private:
    int init();
    void run() override;

    int64_t pts;
    QQueue<PlayData> queue;
    QMutex mutex;
    QMutex ptsMutex;
    struct SwrContext *swr_ctx;
    bool isRun;
};


#endif //PALYER_AUDIODISPOSETHREAD_H

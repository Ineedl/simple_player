//
// Created by cjh on 2024/10/31.
//


//
// Created by cjh on 2024/10/30.
//

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include "AudioDisposeThread.h"

AudioDisposeThread::AudioDisposeThread() {
    this->init();
}

AudioDisposeThread::~AudioDisposeThread() {
    while (isRun){}
    if(swr_ctx!= nullptr){
        swr_free(&swr_ctx);
    }
}

int AudioDisposeThread::init() {
    pts=0;
    isRun= true;
    swr_ctx=nullptr;
    return 0;
}

void AudioDisposeThread::run() {
    qDebug()<<"audio thread id:"<<QThread::currentThreadId();
    while (isRun){
        mutex.lock();
        if(this->queue.isEmpty()){
            mutex.unlock();
            QThread::msleep(100);
        }else{
            PlayData data = queue.dequeue();
            mutex.unlock();
//            qDebug()<<"audio pts:"<<data.pts;
//            if(data.pts<0){
//                data.pts=0;
//            }
//            if(pts==0){
//                pts=data.pts;
//                emit signalAudioPlay(data);
//                continue;
//            }
            int64_t diff=0;
            {
                QMutexLocker tmp(&ptsMutex);
                diff = data.pts-pts;
            }

            if(diff>0){
                QThread::msleep(diff);
            }
//            pts = data.pts;
            emit signalAudioPlay(data);
        }
    }
}


void AudioDisposeThread::slotReceiveAudioData(AVFrame *decode) {
    int64_t pst = av_rescale_q(decode->pts,decode->time_base,AVRational{1,1000});
    do{

        if(swr_ctx== nullptr){
            swr_ctx = swr_alloc_set_opts(NULL,
                                         AV_CH_LAYOUT_STEREO,
                                         AV_SAMPLE_FMT_S16,
                                         8000,
                                         av_get_default_channel_layout(decode->channels),
                                         (AVSampleFormat)decode->format,
                                         decode->sample_rate,
                                         0,
                                         NULL);
            if (!swr_ctx) {
                qDebug()<<"swr ctx get error";
                break;
            }
            if (swr_init(swr_ctx) < 0) {
                qDebug()<<"swr ctx init error";
                swr_free(&swr_ctx);
                swr_ctx= nullptr;
                break;
            }
        }

        uint8_t **converted_samples = NULL;
        if (av_samples_alloc_array_and_samples(&converted_samples,
                                               NULL,
                                               2,
                                               decode->nb_samples,
                                               AV_SAMPLE_FMT_S16,
                                               0) < 0) {
            qDebug()<<"av_samples_alloc_array_and_samples error";
            break;
        }
        int num_samples = swr_convert(swr_ctx,
                                      converted_samples,
                                      decode->nb_samples,
                                      (const uint8_t **) decode->data,
                                      decode->nb_samples);
        if (num_samples < 0 || converted_samples== nullptr || converted_samples[0]== nullptr) {
            qDebug()<<"swr_convert error";
            break;
        }
        int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        {
            QMutexLocker m(&mutex);
            queue.push_back(PlayData{
                    QByteArray(reinterpret_cast<const char *>(converted_samples[0]), num_samples*bytes_per_sample*decode->channels), 0,
                    0, pst,1,num_samples});
        }
        if(converted_samples!= nullptr){
            if(converted_samples[0]!= nullptr){
                av_freep(&converted_samples[0]);
                av_freep(&converted_samples);
            }
        }
    } while (0);
    av_frame_free(&decode);
}

void AudioDisposeThread::slotSetPTS(int64_t outPts){
    QMutexLocker tmp(&ptsMutex);
    pts = outPts;
}

void AudioDisposeThread::stop() {
    isRun = false;
}
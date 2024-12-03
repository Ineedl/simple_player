//
// Created by cjh on 2024/10/30.
//

#include <QDebug>
#include <QDateTime>
#include <QFile>
#include "VideoDisposeThread.h"


VideoDisposeThread::VideoDisposeThread() {
    this->init();
}

VideoDisposeThread::~VideoDisposeThread() {
    while (isRun){}
    if(sws_ctx!= nullptr){
        sws_freeContext(sws_ctx);
    }
    //释放
    if(rgb_frame!= nullptr){
        av_frame_free(&rgb_frame);
    }
}

int VideoDisposeThread::init() {
    pts=0;
    isRun = true;
    rgb_frame     = NULL;
    sws_ctx=nullptr;
    rgb_frame=av_frame_alloc();
    if (!rgb_frame) {
        qDebug()<< "Could not allocate rgb frame";
        return -1;
    }
    return 0;
}

void VideoDisposeThread::run() {
    qDebug()<<"video thread id:"<<QThread::currentThreadId();
    while (isRun){
        mutex.lock();
        if(this->queue.isEmpty()){
            mutex.unlock();
            QThread::msleep(100);
        }else{
            PlayData data = queue.dequeue();
            mutex.unlock();
            if(data.pts<0){
                data.pts=0;
            }
            if(pts==0){
                pts=data.pts;
                emit signalVideoPTSModify(pts);
                emit signalPlay(data);
                continue;
            }
            int64_t diff = data.pts-pts;
            if(diff>0){
                QThread::msleep(diff);
            }
            pts = data.pts;
            emit signalVideoPTSModify(pts);
            emit signalPlay(data);
        }
    }
}


void VideoDisposeThread::slotReceiveVideoData(AVFrame *decode) {
    int64_t pst = av_rescale_q(decode->pts,decode->time_base,AVRational{1,1000});
    do{
            int width = decode->width;
            int height = decode->height;
            int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24,width ,height, 1);
            uint8_t *buffer = (uint8_t *)av_malloc(num_bytes);

            // 将数据填充到 rgb_frame 中
            av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, width, height, 1);

            if(sws_ctx== nullptr){
                sws_ctx = sws_getContext(
                        width, height, (AVPixelFormat)decode->format,
                        width, height, AV_PIX_FMT_RGB24,
                        SWS_BILINEAR, NULL, NULL, NULL);
                if(sws_ctx== nullptr){
                    qDebug()<<"sws_ctx is null";
                    break;
                }
            }

            // 3. 执行转换
            int ret = sws_scale(sws_ctx, decode->data, decode->linesize, 0, height,
                                rgb_frame->data, rgb_frame->linesize);
            if(ret<=0) {
                qDebug() << "sws err";
                break;
            }
            {
                QMutexLocker m(&mutex);
                queue.push_back(PlayData{
                        QByteArray(reinterpret_cast<const char *>(rgb_frame->data[0]), rgb_frame->linesize[0] * height), width,
                        height, pst,0});
            }
            memset(buffer,0,num_bytes);
            av_free(buffer);
    } while (0);
    av_frame_free(&decode);
}

void VideoDisposeThread::stop() {
    isRun = false;
}
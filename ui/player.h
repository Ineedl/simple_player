//
// Created by cjh on 2024/10/29.
//

#ifndef PALYER_PLAYER_H
#define PALYER_PLAYER_H

#include <QWidget>
#include <QThread>
#include <QAudioOutput>
#include "AudioDisposeThread.h"
#include "ReceiveThread.h"
#include "VideoDisposeThread.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Player; }
QT_END_NAMESPACE

class Player : public QWidget {
Q_OBJECT



public:
    explicit Player(QWidget *parent = nullptr);

    ~Player() override;

public slots:
    void slotPlay(PlayData data);

private:
    void init();
    int initConnect();

private:
    Ui::Player *ui;
    ReceiveThread recvThread;
    VideoDisposeThread videoThread;
    AudioDisposeThread audioThread;
    QAudioFormat format;
    QIODevice *device;
    QAudioOutput* output;

};


#endif //PALYER_PLAYER_H

//
// Created by cjh on 2024/10/29.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Player.h" resolved

#include "player.h"
#include "ui_Player.h"
#include <QDebug>


Player::Player(QWidget *parent) :
        QWidget(parent), ui(new Ui::Player) {
    ui->setupUi(this);
    this->init();
}

Player::~Player() {
    recvThread.stop();
    videoThread.stop();
    audioThread.stop();
    device->close();
    delete ui;
    delete output;
    delete device;
}

void Player::init() {
    format.setCodec("audio/pcm");
    format.setSampleRate(8000);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setChannelCount(2);
    format.setByteOrder(QAudioFormat::LittleEndian);

    output=new QAudioOutput(format, qApp);
    device = output->start();

    this->initConnect();

    this->ui->label->setText("");
    this->recvThread.start();
    this->videoThread.start();
    this->audioThread.start();
}

void Player::slotPlay(PlayData showData) {
    if(showData.type==0){
        QImage image(reinterpret_cast<unsigned char *>(showData.data.data()), showData.width, showData.height, QImage::Format_RGB888);
        this->ui->label->setPixmap(QPixmap::fromImage(image));
        this->ui->label->update();
    }else{
        int readSize = output->periodSize();
        int chunks = output->bytesFree() / readSize;
        if(showData.data.size()>0){
            while (chunks) {
                QByteArray samples = showData.data.mid(0, readSize);
                int len = samples.size();
                showData.data.remove(0, len);
                if (len) device->write(samples);
                if (len != readSize) break;
                chunks--;
            }
        }
    }
}

int Player::initConnect() {
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<PlayData>("PlayData");
    connect(&this->recvThread, &ReceiveThread::signalVideoSendData, &this->videoThread, &VideoDisposeThread::slotReceiveVideoData, Qt::DirectConnection);
    connect(&this->recvThread,&ReceiveThread::signalAudioSendData,&this->audioThread,&AudioDisposeThread::slotReceiveAudioData,Qt::DirectConnection);
    connect(&this->videoThread, &VideoDisposeThread::signalVideoPTSModify, &this->audioThread, &AudioDisposeThread::slotSetPTS, Qt::DirectConnection);
    connect(&this->videoThread, &VideoDisposeThread::signalPlay, this, &Player::slotPlay);
    connect(&this->audioThread,&AudioDisposeThread::signalAudioPlay, this,&Player::slotPlay);
}




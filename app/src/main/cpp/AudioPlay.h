//
// Created by xb on 2020/6/10.
//

#ifndef MINEFFMPEG_AUDIOPLAY_H
#define MINEFFMPEG_AUDIOPLAY_H

#include "Parameter.h"
#include "Data.h"
#include "IFFBase.h"
#include <list>
#include <jni.h>




class AudioPlay : public IFFBase {
public:
    AudioPlay();

    virtual ~AudioPlay();

    virtual bool init(Parameter out);

    virtual void close();

    //清理清理音频采样后的数据队列
    virtual void clearResample();

    //最大缓冲
    int maxFrame = 100;

    //播放回调方法
    bool playCall(void *bufq);

    //如果 playCall()终止回调 就用该方法尝试激活
    void reactivatePlayCall();

    //获取缓冲数据，如没有则阻塞
    virtual Data getResampleData();

    std::list<Data> resampleFrameList;

protected:
    unsigned char *buf = 0;
};

#endif //MINEFFMPEG_AUDIOPLAY_H

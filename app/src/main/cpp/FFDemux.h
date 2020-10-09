//
// Created by xb on 2020/6/17.
// 解封装
//

#ifndef MINEFFMPEG_FFDEMUX_H
#define MINEFFMPEG_FFDEMUX_H

#include "Parameter.h"
#include "Data.h"
#include "IFFBase.h"

class FFDemux :public IFFBase{
public:
    FFDemux(void);

    //打开文件，或者流媒体 rmtp http rtsp(解协议，解封装)
    virtual bool open(const char *url);

    //seek 位置 pos 0.0~1.0
    virtual bool seek(double pos);

    //获取视频参数
    virtual Parameter getVideoPara();

    //获取音频参数
    virtual Parameter getAudioPara();

    //读取一帧数据，数据由调用者清理
    virtual Data readAVFormatData();

    virtual void close();
};
#endif //MINEFFMPEG_FFDEMUX_H

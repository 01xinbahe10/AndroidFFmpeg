//
// Created by xb on 2020/6/17.
//

#ifndef MINEFFMPEG_FFRESAMPLE_H
#define MINEFFMPEG_FFRESAMPLE_H

#include "IFFBase.h"

class FFResample : public IFFBase{

public:
    virtual bool resample(Parameter in,Parameter out=Parameter());

    //从线程中获取解码结果  再次调用会复用上次空间，线程不安全
    virtual Data readResample(Data indata) ;

    virtual void closeResample();

};
#endif //MINEFFMPEG_FFRESAMPLE_H

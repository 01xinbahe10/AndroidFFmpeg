//
// Created by xb on 2020/6/17.
//解码
//

#ifndef MINEFFMPEG_FFDECODE_H
#define MINEFFMPEG_FFDECODE_H

#include "IFFBase.h"

class FFDecode : public IFFBase{
public:
    //解码
    virtual bool decoding(Parameter para, bool isHard);

    //future模型 发送数据到线程解码
    virtual bool sendPacket(Data pkt);

    //从线程中获取解码结果，再次调用会复用上次空间，线程不安全
    virtual Data recvFrame();

    virtual void clearAVCodec();

    virtual void closeAVCodec();

};
#endif //MINEFFMPEG_FFDECODE_H

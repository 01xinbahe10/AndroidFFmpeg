//
// Created by xb on 2020/6/17.
//

#ifndef MINEFFMPEG_IFFBASE_H
#define MINEFFMPEG_IFFBASE_H
#include "Data.h"
#include "Parameter.h"
#include <list>
#include "Log.h"

struct AVFormatContext;//AVFormat 解封装的上下文
struct AVCodecParameters;//AVCodec 解封装的参数
struct AVFrame;//画帧
struct AVCodecContext;//AVCodec 解码的上下文
struct SwrContext;//重采样的上下文
struct VideoView;//图像流

class IFFBase{
public:
    AVFormatContext *formatContext = 0;

    //画面参数流
    int videoStream = 0;
    AVCodecContext *avCodecContext = 0;
    AVFrame *avFrame = 0;
    //音频参数流
    int audioStream = 1;
    SwrContext *swrContext = 0;

};
#endif //MINEFFMPEG_IFFBASE_H

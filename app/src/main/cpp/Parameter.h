
#ifndef XPLAY_XPARAMETER_H
#define XPLAY_XPARAMETER_H


#include "PlayUtil.h"

struct AVCodecParameters;

class Parameter {
public:
    AVCodecParameters *para = 0;
    int channels = PlayUtil::get()->audioChannel;//默认
    int sample_rate = PlayUtil::get()-> audioSampleRate;//默认
};


#endif //XPLAY_XPARAMETER_H

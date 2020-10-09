//
// Created by xb on 2020/6/17.
//


#include "FFResample.h"
#include "Log.h"
extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}


bool FFResample::resample(Parameter in, Parameter out) {
    closeResample();
    //音频重采样上下文初始化
    swrContext = swr_alloc();
    swrContext = swr_alloc_set_opts(swrContext,
                                    av_get_default_channel_layout(in.para->channels),
                                    AV_SAMPLE_FMT_S16,in.para->sample_rate,
                                    av_get_default_channel_layout(in.para->channels),
                                    (AVSampleFormat)in.para->format,in.para->sample_rate,
                                    0,0 );

    int re = swr_init(swrContext);
    if(re != 0){
        LOGE("swr_init failed!");
        return false;
    }else{
        LOGI("swr_init success!");
    }

    PlayUtil::get()->resampleOutFormat = AV_SAMPLE_FMT_S16;
    PlayUtil::get()->audioChannel = in.para->channels;
    PlayUtil::get()->audioSampleRate = in.para->sample_rate;
    //    LOGE(">>>>>>>>>>>>  通道1：%d   采样率1：%d",audio_channel,audio_sample_rate);
//    LOGE(">>>>>>>>>>>>  通道2：%d   采样率2：%d",audio_in_channel,audio_in_sample_rate);
    return true;
}


Data FFResample::readResample(Data indata) {
    if (indata.size <= 0 || !indata.data) {
        return Data();
    }
    if (!swrContext) {
        return Data();
    }

    //LOGE("indata pts is %d",indata.pts);
    AVFrame *frame = (AVFrame *) indata.data;

    //输出空间的分配
    Data out;
    int outsize = PlayUtil::get()->audioChannel * frame->nb_samples *
                  av_get_bytes_per_sample((AVSampleFormat) PlayUtil::get()->resampleOutFormat);
    if (outsize <= 0) {
        return Data();
    }
    out.Alloc(outsize);
    uint8_t *outArr[2] = {0};
    outArr[0] = out.data;
    int len = swr_convert(swrContext, outArr, frame->nb_samples, (const uint8_t **) frame->data,
                          frame->nb_samples);
    if (len <= 0) {
        out.Drop();
        return Data();
    }
    out.pts = indata.pts;
    //音频帧率 =  一秒/((一帧采样数*1000)/音频采样率)
    double frame_rate = 1000/(frame->nb_samples*1000.0/PlayUtil::get()->audioSampleRate);
    if (frame_rate != PlayUtil::get()->audioFrameRate){
        PlayUtil::get()->audioFrameRate = frame_rate;
    }
//    LOGE("音帧率:%d   一帧采样数:%d    采样率：%d",audio_frame_rate,frame->nb_samples,audio_out_sample_rate);
//    out.size = indata.size;
//    out.format = indata.format;
//    LOGE("swr_convert success = %d",len);
    return out;
}

void FFResample::closeResample() {
    if(swrContext){
        swr_free(&swrContext);
    }
}
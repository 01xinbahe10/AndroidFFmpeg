//
// Created by xb on 2020/6/17.
//


#include "FFDecode.h"
#include "Log.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

bool FFDecode::decoding(Parameter para, bool isHard) {
    closeAVCodec();
    if (!para.para) {
        return false;
    }
    //1 查找解码器
    AVCodec *cd = avcodec_find_decoder(para.para->codec_id);
    if (isHard) {
        cd = avcodec_find_decoder_by_name("h264_mediacodec");
    }
    if (!cd) {
        LOGE("avcodec_find_decoder %d failed!  %d", para.para->codec_id, isHard);
        return false;
    }
    LOGI("avcodec_find_decoder success %d!", isHard);

    //2 创建解码上下文，并复制参数
    avCodecContext = avcodec_alloc_context3(cd);
    avcodec_parameters_to_context(avCodecContext,para.para);

    avCodecContext->thread_count = 3;
    //3 打开解码器
    int codec_status = avcodec_open2(avCodecContext,0,0);
    if(codec_status != 0){
        char buf[1024] = {0};
        av_strerror(codec_status,buf,sizeof(buf)-1);
        LOGE("%s",buf);
        return false;
    }

//    if(avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO){
//        this->isAudio = false;
//    }else{
//        this->isAudio = true;
//    }

    LOGI("avcodec_open2 success!");
    return true;

}


void FFDecode::clearAVCodec(){
    if(avCodecContext){
        avcodec_flush_buffers(avCodecContext);
    }
}

bool FFDecode::sendPacket(Data pkt) {
    if (pkt.size <= 0 || !pkt.data) {
        return false;
    }
    if (!avCodecContext) {
        return false;
    }
    int re = avcodec_send_packet(avCodecContext, (AVPacket *) pkt.data);

    if (re != 0) {
//        av_packet_unref((AVPacket *) pkt.data);
        return false;
    }

    return true;
}


Data FFDecode::recvFrame() {
    if (!avCodecContext) {
        return Data();
    }
    if (!avFrame) {
        avFrame = av_frame_alloc();
    }
    int re = avcodec_receive_frame(avCodecContext, avFrame);
    if (re != 0) {
        return Data();
    }
    Data d;
    d.data = (unsigned char *) avFrame;
    if (avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
        d.size = (avFrame->linesize[0] + avFrame->linesize[1] + avFrame->linesize[2]) *
                 avFrame->height;
        d.width = avFrame->width;
        d.height = avFrame->height;
    } else {
        //样本字节数 * 单通道样本数 * 通道数
        d.size = av_get_bytes_per_sample((AVSampleFormat) avFrame->format) * avFrame->nb_samples * 2;
    }
    d.format = avFrame->format;
    //if(!isAudio)
    //    XLOGE("data format is %d",frame->format);
    memcpy(d.datas, avFrame->data, sizeof(d.datas));
    d.pts = avFrame->pts;
//    pts = d.pts;
    return d;
}

void FFDecode::closeAVCodec() {
    if (avFrame) {
        //释放画帧
        av_frame_free(&avFrame);
    }
    if (avCodecContext) {
        //释放解码上下文
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
    }
}


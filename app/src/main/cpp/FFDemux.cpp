//
// Created by xb on 2020/6/17.
//

#include "FFDemux.h"
#include "Log.h"
#include "PlayUtil.h"

using namespace std;
extern "C" {
#include <libavformat/avformat.h>
}


//分数转为浮点数（计算帧时）
static double r2d(AVRational r) {
//    LOGE("---------1----------r.num:%d  r.den:%d",r.num,r.den);
    return r.num == 0 || r.den == 0 ? 0. : (double) r.num / (double) r.den;
}

FFDemux::FFDemux() {
    //注册所有封装器
    av_register_all();

    //注册所有的解码器
    avcodec_register_all();

    //初始化网络
    avformat_network_init();
}

bool FFDemux::open(const char *url) {
    close();
    LOGI("ffcontrol url:%s", url);
    int open_status = avformat_open_input(&formatContext, url, 0, 0);
    if (open_status != 0) {
        char buf[1024] = {0};
        av_strerror(open_status, buf, sizeof(buf));
        LOGE("ffcontrol avformat_open_input open failed! url:%s   %d", buf,open_status);
        return false;
    }

    //读取文件信息
    open_status = avformat_find_stream_info(formatContext, 0);
    if (open_status != 0) {
        char buf[1024] = {0};
        av_strerror(open_status, buf, sizeof(buf));
        LOGE("ffcontrol avformat_find_stream_info open failed! url:%s", url);
        return false;
    }

    if (!formatContext) {
        LOGE("ffcontrol AVFormatContext null! ");
        return false;
    }

    PlayUtil::get()->totalTime = formatContext->duration / (AV_TIME_BASE / 1000.0);
    LOGE("总时长--------------》      %f",PlayUtil::get()->totalTime);

    //获取视频参数
    //获取了视频流索引
    getVideoPara();

    //获取音频参数
    //获取了音频流索引
    getAudioPara();
    LOGI("av_find_best_stream success!");
    return true;
}

bool FFDemux::seek(double pos) {
    if (pos < 0 || pos > 1) {
        LOGE("Seek value must 0.0~1.0");
        return false;
    }
    bool re = false;
    if (!formatContext) {
        return false;
    }
    //清理读取的缓冲
    avformat_flush(formatContext);
    long long seekPts = 0;
    seekPts = formatContext->streams[videoStream]->duration * pos;

    //往后跳转到关键帧
    re = av_seek_frame(formatContext, videoStream, seekPts,
                       AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    return re;
}

Parameter FFDemux::getVideoPara() {
    if (!formatContext) {
        LOGE("GetVPara failed! ic is NULL！");
        return Parameter();
    }
    //获取了视频流索引
    int re = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
    if (re < 0) {
        LOGE("av_find_best_stream failed!");
        return Parameter();
    }
    videoStream = re;
    Parameter para;
    para.para = formatContext->streams[re]->codecpar;

    return para;
}

Parameter FFDemux::getAudioPara() {
    if (!formatContext) {
        LOGE("GetVPara failed! ic is NULL！");
        return Parameter();
    }
    //获取了音频流索引
    int re = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, 0, 0);
    if (re < 0) {
        LOGE("av_find_best_stream failed!");
        return Parameter();
    }
    audioStream = re;
    Parameter para;
    para.para = formatContext->streams[re]->codecpar;
    para.channels = formatContext->streams[re]->codecpar->channels;
    para.sample_rate = formatContext->streams[re]->codecpar->sample_rate;
    return para;
}


Data FFDemux::readAVFormatData() {
    if (!formatContext) {
        return Data();
    }
    Data d;
    AVPacket *pkt = av_packet_alloc();
    int re = av_read_frame(formatContext, pkt);
    if (re != 0) {
        av_packet_free(&pkt);
        return Data();
    }
    //XLOGI("pack size is %d ptss %lld",pkt->size,pkt->pts);
    d.data = (unsigned char *) pkt;
    d.size = pkt->size;
    AVStream *stream = formatContext->streams[pkt->stream_index];
    if (pkt->stream_index == audioStream) {
        d.stream_index = STREAM_AUDIO;
//        LOGI("音频time_base  num:%d  den:%d  pts:%ld",stream->time_base.num,stream->time_base.den,pkt->pts);
    } else if (pkt->stream_index == videoStream) {
        d.stream_index = STREAM_VIDEO;
//        LOGE("视频time_base  num:%d  den:%d  pts:%ld",stream->time_base.num,stream->time_base.den,pkt->pts);
        int unitFrame = stream->avg_frame_rate.num;
        int unitOfTime = stream->avg_frame_rate.den;
        double frame_rate = unitFrame / (double) unitOfTime;
        if (frame_rate != PlayUtil::get()->videoFrameRate) {
            PlayUtil::get()->videoFrameRate = frame_rate;
        }
//        LOGE("视帧率:%d  单位帧(帧数):%d  单位时间(秒):%d",frame_rate,unitFrame,unitOfTime);
    } else {
        av_packet_free(&pkt);
        return Data();
    }

    //转换pts
//    LOGE("---------0----------pkt->pts  %d",pkt->pts);
//    LOGE("demux pts %f",(pkt->pts * r2d(formatContext->streams[pkt->stream_index]->time_base)));
//    LOGE("---------5---------  %f",(formatContext->streams[pkt->stream_index]->duration * r2d(formatContext->streams[pkt->stream_index]->time_base)));
//    pkt->pts = pkt->pts * (1000 * r2d(formatContext->streams[pkt->stream_index]->time_base));
//    pkt->dts = pkt->dts * (1000 * r2d(formatContext->streams[pkt->stream_index]->time_base));
//
//    LOGE("---------3----------pkt->pts  %d",pkt->pts);
//    d.pts = (int) pkt->pts;
//    LOGE("---------4----------d.pts  %d",d.pts);

    AVRational time_base = stream->time_base;
    pkt->pts = pkt->pts * (1000.0 * r2d(time_base));
    pkt->dts = pkt->dts * (1000.0 * r2d(time_base));
    d.pts = pkt->pts;
    return d;
}


void FFDemux::close() {
    if (formatContext) {
        avformat_close_input(&formatContext);
        formatContext = 0;
        LOGI("ffcontrol close");
    }
}
//
// Created by xb on 2020/6/11.
//

#ifndef MINEFFMPEG_FFPLAYER_H
#define MINEFFMPEG_FFPLAYER_H


#include <jni.h>
#include <condition_variable>
#include "VideoView.h"
#include "FFDemux.h"
#include "FFDecode.h"
#include "FFResample.h"
#include "AudioPlay.h"




class FFPlayer :public Observer{
private:
    // 禁止外部构造
    FFPlayer();
    //禁止外部析构
    ~FFPlayer();
    //禁止外部复制构造
    FFPlayer(const FFPlayer &ffplay);
    //禁止外部复制操作
    const FFPlayer &operator = (const FFPlayer &ffplay);

    static const FFPlayer *ffplay;
public:
    static FFPlayer *Get();

    virtual void playerCallBack(int thread_id,int status);


    pthread_mutex_t mutex; //线程锁
    pthread_cond_t cond;


    //解封装
    FFDemux *ffDemux = &ffDemux1;
    pthread_t avFormatPt;

    //图片解码及绘制
    FFDecode *ffVideoDecode = &ffVideoDecode1;
    pthread_t videoPlayPt;
    VideoView *videoView = &videoView1;

    //音频解码及重采样
    FFDecode *ffAudioDecode = &ffAudioDecode1;
    pthread_t audioPlayPt;
    FFResample *ffResample = &ffResample1;
    AudioPlay *audioPlay = &audioPlay1;



    //视图解码缓冲
    std::list<Data> decodeVideoList;
    //音频解码缓冲
    std::list<Data> decodeAudioList;
    double pts = 0;

    //清理音画包缓冲队列
    void clearPacket(){
//        decodeVideoList.clear();
//        decodeAudioList.clear();
        while (!decodeVideoList.empty()){
            decodeVideoList.front().Drop();
            decodeVideoList.pop_front();
        }
        while (!decodeAudioList.empty()){
            decodeAudioList.front().Drop();
            decodeAudioList.pop_front();
        }

    }


    int decode_video(FFPlayer *player,Data *data);
    int decode_audio(FFPlayer *player,Data *data);

    void restartClear();//重新开始时，需要清理某一部分数据


private:
    //解封装
    FFDemux ffDemux1;
    //图片解码
    FFDecode ffVideoDecode1;
    //绘制窗口
    VideoView videoView1;
    //音频解码
    FFDecode ffAudioDecode1;
    //重采样
    FFResample ffResample1;
    //音频播放
    AudioPlay audioPlay1;
};


#endif //MINEFFMPEG_FFPLAYER_H

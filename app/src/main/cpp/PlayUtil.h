//
// Created by xb on 2020/7/23.
//

#ifndef MINEFFMPEG_PLAYUTIL_H
#define MINEFFMPEG_PLAYUTIL_H

#include <time.h>
#include <unistd.h>
#include "Observer.h"

//#define NULL_VALUE  -1

//获取当前时间
static timespec time1;
static double getCurrentTime() {
    clock_gettime(CLOCK_REALTIME, &time1);
    return (time1.tv_sec * 1000.0) + (time1.tv_nsec / 1000000.0);
}

enum STATE{
    NULL_VALUE = -1,//空值

    VIDEO_NO_CACHE = 100,//视频没有缓存
    VIDEO_CAN_PLAY = 101,//视频可以播放

    AUDIO_NO_CACHE = 110,//音频没有缓存
    AUDIO_CAN_PLAY = 111,//音频可以播放
    AUDIO_PLAYER_LOOP_STOP = 112,//音频播放器循环停止

};

class PlayUtil{

// 内部静态变量的懒汉单例
private:
    // 禁止外部构造
    PlayUtil();
    //禁止外部析构
    ~PlayUtil();
    //禁止外部复制构造
    PlayUtil(const PlayUtil &_playUtil);
    //禁止外部复制操作
    const PlayUtil &operator = (const PlayUtil &_playUtil);

    static const PlayUtil *_util;



public:
    // 获取单实例对象
    static PlayUtil * get();
    // 打印实例地址
    void print();

    //////////////////////画面和音频参数///////////////////////////
    double totalTime = 0;//总时长
    /**画面状态*/
    double videoFrameRate = 0;//画帧率(表示一秒中播放多少帧)
    double videoOldPlayTime = NULL_VALUE;
    double videoDrawDiff = NULL_VALUE;//绘制的时间
    double vpts = 0;

    /*
     * 表示画面准备就绪是否可以播放（需要撘配 can_play_audio 一起使用）
     * 1:校对起始播放时，音画几乎同时播放
     * 2:检测音画缓存是否同时有，一方没有缓存就暂停
     * */
    bool canPlayVideo = false;

    /**音频状态*/
    double audioFrameRate = 0;//音帧率
    int audioChannel = 2;//音频声频通道（默认）
    int audioSampleRate = 44100;//音频采样率（默认）
    int resampleOutFormat = 1;
    double audioPlayCurrentTime = NULL_VALUE;
    double audioOldPlayTime = NULL_VALUE;
    double audioPlayDiff = NULL_VALUE;//音频播放的时间
    bool canPlayAudio = false;//表示音频准备就绪是否可以播放
    bool audioPlayerCall = false;//表示音频回调方法是否正常回调
    double apts = NULL_VALUE;



    /////////////////////////播放状态////////////////////////////////
    //当前播放的地址(播放源)
    const char *playbackSource = 0;

    bool isRunFormat = false;

    bool isRunPlayVideo = false;

    bool isRunListener = false;

    bool isPlayPause = false;//播放是否暂停

    bool switchPlaySource = false;//是否切换了播放源

    Observer *observer;


//公共方法区
public:
    //针对暂停时做延时处理，防止死循环时占用cpu率高
     void playPauseSleep(){
        long delay  = 5*1000*100;//500毫秒
        usleep(delay);//单位是微秒
    }

    void noDataSleep(){
        long delay  = 2*1000;//2毫秒
        usleep(delay);//单位是微秒
    }

};

#endif //MINEFFMPEG_PLAYUTIL_H

//
// Created by xb on 2020/6/10.
//

#include "AudioPlay.h"
#include "FFPlayer.h"
#include "PlayUtil.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <unistd.h>

extern "C" {
#include <libavutil/time.h>
}

static SLObjectItf engineSL = NULL;
static SLEngineItf eng = NULL;
static SLObjectItf mix = NULL;
static SLObjectItf player = NULL;
static SLPlayItf iplayer = NULL;
static SLAndroidSimpleBufferQueueItf pcmQue = NULL;


//音频缓冲区的大小
static int s_audioCacheSize = 1;
double g_audioCurrentTime;
bool  g_audioIsLock = false;
/*
 * 表示静音音频播放的累计时;当操过一定的静音音频播放时间就关闭播放回调;
 * 静音音频表示：没有音频数据时而创建了一个静音音频数据进行播放，
 * 这样目的是防止音频播放器的回调断档和音频播放的连续性
 * 单位：毫秒
 * 作用：防止音频播放器的回调一直运行，消耗性能和浪费电
 * */
double g_muteAudioStartTime = 0;//静音音频播放的开始时间
const double g_muteAudioCumulativeTime = 5000;//限时5秒后关闭音频播放器回调

AudioPlay::AudioPlay() {
    buf = new unsigned char[s_audioCacheSize];
}

AudioPlay::~AudioPlay() {
    delete buf;
    buf = 0;
}

static SLEngineItf createSL() {

    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL, 0, 0, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (re != SL_RESULT_SUCCESS) return NULL;
    return en;
}

static void pcmCall(SLAndroidSimpleBufferQueueItf bf, void *contex) {
    AudioPlay *ap = (AudioPlay *) contex;
    if (!ap) {
        LOGE("PcmCall failed contex is null!");
        PlayUtil::get()->audioPlayerCall = false;
        return;
    }
    PlayUtil::get()->audioPlayerCall = ap->playCall((void *) bf);
}


bool AudioPlay::playCall(void *bufq) {
//    LOGE("播放循环走走走走");
    if (!bufq) {
        return false;
    }

    SLAndroidSimpleBufferQueueItf bf = (SLAndroidSimpleBufferQueueItf) bufq;

    pthread_mutex_lock(&FFPlayer::Get()->mutex);
    Data d = getResampleData();
    pthread_cond_broadcast(&FFPlayer::Get()->cond);
    pthread_mutex_unlock(&FFPlayer::Get()->mutex);


    if (d.size <= 0) {
        pthread_mutex_lock(&FFPlayer::Get()->mutex);
        PlayUtil::get()->apts = NULL_VALUE;
        if (g_muteAudioStartTime == 0){
            g_muteAudioStartTime = getCurrentTime();
        } else if (getCurrentTime() - g_muteAudioStartTime > g_muteAudioCumulativeTime) {
            pthread_cond_broadcast(&FFPlayer::Get()->cond);
            pthread_mutex_unlock(&FFPlayer::Get()->mutex);
            return false;
        }
        pthread_cond_broadcast(&FFPlayer::Get()->cond);
        pthread_mutex_unlock(&FFPlayer::Get()->mutex);

//        LOGE("GetData(1) size is 0");
        // 音频解码错误，播放静音
        //将数组buf中的元素都赋值为0,大小为d.size;相当于初始化赋值
        memset(buf, 0, s_audioCacheSize);
        if (*bf) {
            (*bf)->Enqueue(bf, buf, s_audioCacheSize);
        }


    } else {
        g_muteAudioStartTime = 0;
        pthread_mutex_lock(&FFPlayer::Get()->mutex);
        PlayUtil::get()->apts = d.pts;
        PlayUtil::get()->audioPlayCurrentTime = getCurrentTime();
        pthread_cond_broadcast(&FFPlayer::Get()->cond);
        pthread_mutex_unlock(&FFPlayer::Get()->mutex);

        if (s_audioCacheSize != d.size) {
            s_audioCacheSize = d.size;
            if (buf) {
                delete buf;
                buf = 0;
            }
            buf = new unsigned char[s_audioCacheSize];
        }
        //将d.data中数据复制到缓存buf中
        memcpy(buf, d.data, s_audioCacheSize);
        if (*bf) {
            (*bf)->Enqueue(bf, buf, s_audioCacheSize);
//            LOGI("音频播放时间：%f   帧时：%f", getCurrentTime(), d.pts);
        }
    }

    if (d.data) {
        d.Drop();
    }

    return true;
}

void AudioPlay::reactivatePlayCall() {
    //长度要和(buf)缓存长度一样
    //将数组buf中的元素都赋值为0,大小为d.size;相当于初始化赋值
    //用空的语音启动
    memset(buf, 0, s_audioCacheSize);
    if (pcmQue && (*pcmQue)) {
        (*pcmQue)->Enqueue(pcmQue, buf, s_audioCacheSize);
    }
    LOGI("reactivatePlayCall----------------------------");

}


Data AudioPlay::getResampleData() {
//    LOGI("音频数据222》》》  %d",frames.size());
    if(PlayUtil::get()->isPlayPause){
        return Data();
    }
    if (!resampleFrameList.empty()) {
        PlayUtil::get()->canPlayAudio = true;//表示可以播放音频了
        if(!PlayUtil::get()->canPlayVideo){
            return Data();
        }
        //有数据返回
        Data d = resampleFrameList.front();
        resampleFrameList.pop_front();
        return d;
    }
    PlayUtil::get()->canPlayAudio = false;
    //未获取数据
    return Data();
}

void AudioPlay::clearResample() {
//    frames.clear();
    while (!resampleFrameList.empty()) {
        resampleFrameList.front().Drop();
        resampleFrameList.pop_front();
    }
}

void AudioPlay::close() {

    //停止播放
    if (iplayer && (*iplayer)) {
        (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_STOPPED);
    }
    //清理播放队列
    if (pcmQue && (*pcmQue)) {
        (*pcmQue)->Clear(pcmQue);
    }
    //销毁player对象
    if (player && (*player)) {
        (*player)->Destroy(player);
    }
    //销毁混音器
    if (mix && (*mix)) {
        (*mix)->Destroy(mix);
    }

    //销毁播放引擎
    if (engineSL && (*engineSL)) {
        (*engineSL)->Destroy(engineSL);
    }

    engineSL = NULL;
    eng = NULL;
    mix = NULL;
    player = NULL;
    iplayer = NULL;
    pcmQue = NULL;
}

bool AudioPlay::init(Parameter out) {
    clearResample();
    close();
    //1 创建引擎
    eng = createSL();
    if (eng) {
        LOGI("CreateSL success！ ");
    } else {
        LOGE("CreateSL failed！ ");
        return false;
    }

    //2 创建混音器

    SLresult re = 0;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("SL_RESULT_SUCCESS failed!");
        return false;
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("(*mix)->Realize failed!");
        return false;
    }
    SLDataLocator_OutputMix outmix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outmix, 0};

    //3 配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    //音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            static_cast<SLuint32>(PlayUtil::get()->audioChannel),//    声道数
//            SL_SAMPLINGRATE_44_1,//采用率44100
            static_cast<SLuint32>(PlayUtil::get()->audioSampleRate*1000),//采用率(默认44100)
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN //字节序，小端
    };
    SLDataSource ds = {&que, &pcm};


    //4 创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng, &player, &ds, &audioSink,
                                   sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer failed!");
        return false;
    } else {
        LOGI("CreateAudioPlayer success!");
    }
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    //获取player接口
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iplayer);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("GetInterface SL_IID_PLAY failed!");
        return false;
    }
    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("GetInterface SL_IID_BUFFERQUEUE failed!");
        return false;
    }

    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue, pcmCall, this);

    //设置为播放状态
    (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PLAYING);

    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue, "", 1);
    LOGI("SLAudioPlay::StartPlay success!");
    return true;
}
//
// Created by xb on 2020/6/11.
//

#include <jni.h>
#include <pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <threads.h>
#include <android/native_window_jni.h>
#include <SLES/OpenSLES_Android.h>
#include <iostream>

#include "FFPlayer.h"
#include "Log.h"
#include "Shader.h"
#include "PlayUtil.h"


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}
using namespace std;



//全局存放java虚拟机对象
JavaVM *g_javaVM;
ANativeWindow * g_win;
jclass g_ffmpegJni;//Java类
jmethodID g_callPlayStatus;//Java方法函数名

double g_currentTime;
bool g_isLock;


extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_javaVM = vm;

    return JNI_VERSION_1_6;
}

FFPlayer::FFPlayer() {
    PlayUtil::get()->observer = this;
}

FFPlayer::~FFPlayer() {
}


FFPlayer *FFPlayer::Get() {
    static FFPlayer ffPlayer;
    return &ffPlayer;
}

void FFPlayer::restartClear() {
    PlayUtil::get()->isPlayPause = true;//先暂停播放

    audioPlay->clearResample();
    clearPacket();
    ffVideoDecode->clearAVCodec();
    ffAudioDecode->clearAVCodec();

    ffVideoDecode->closeAVCodec();
    ffAudioDecode->closeAVCodec();
    ffDemux->close();

    PlayUtil::get()->isPlayPause = false;//清理完了就可以播放

    LOGI("清理完成");
}


int FFPlayer::decode_audio(FFPlayer *player, Data *data) {
    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);

    if (avcodec_send_packet(player->ffAudioDecode->avCodecContext,(AVPacket *) data->data) == 0) {
        if (!player->ffAudioDecode->avFrame) {
            player->ffAudioDecode->avFrame = av_frame_alloc();
        }
        if (avcodec_receive_frame(player->ffAudioDecode->avCodecContext,player->ffAudioDecode->avFrame) == 0) {

            Data d;
            d.data = (unsigned char *) player->ffAudioDecode->avFrame;
            //样本字节数 * 单通道样本数 * 通道数
            d.size = av_get_bytes_per_sample(
                    (AVSampleFormat) player->ffAudioDecode->avFrame->format) *
                     player->ffAudioDecode->avFrame->nb_samples * PlayUtil::get()->audioChannel;
            d.format = player->ffAudioDecode->avFrame->format;
            d.pts = player->ffAudioDecode->avFrame->pts;
            memcpy(d.datas, player->ffAudioDecode->avFrame->data, sizeof(d.datas));
            Data resample = player->ffResample->readResample(d);



            pthread_mutex_lock(&player->mutex);
            if (!PlayUtil::get()->audioPlayerCall){
                player->audioPlay->reactivatePlayCall();
            }
            player->audioPlay->resampleFrameList.push_back(resample);
            PlayUtil::get()->audioOldPlayTime = getCurrentTime();
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);


            pthread_mutex_lock(&FFPlayer::Get()->mutex);
            av_frame_free(&player->ffAudioDecode->avFrame);
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);

        }
        return 0;
    }
    return -1;
}

int FFPlayer::decode_video(FFPlayer *player, Data *data) {

    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);

    int re = avcodec_send_packet(player->ffVideoDecode->avCodecContext,(AVPacket *) data->data);
    if (re == 0) {
        if (!player->ffVideoDecode->avFrame) {
            player->ffVideoDecode->avFrame = av_frame_alloc();
        }

        re = avcodec_receive_frame(player->ffVideoDecode->avCodecContext,player->ffVideoDecode->avFrame);
        if (re == 0) {
            Data d;
            d.data = (unsigned char *) player->ffVideoDecode->avFrame;
            d.size = (player->ffVideoDecode->avFrame->linesize[0] +
                      player->ffVideoDecode->avFrame->linesize[1] +
                      player->ffVideoDecode->avFrame->linesize[2]) *
                     player->ffVideoDecode->avFrame->height;
            d.width = player->ffVideoDecode->avFrame->width;
            d.height = player->ffVideoDecode->avFrame->height;
            d.format = player->ffVideoDecode->avFrame->format;
            d.pts = player->ffVideoDecode->avFrame->pts;
            memcpy(d.datas, player->ffVideoDecode->avFrame->data, sizeof(d.datas));

            pthread_mutex_lock(&player->mutex);
            PlayUtil::get()->canPlayVideo = true;//表示视频准备就绪可以播放了
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);


            pthread_mutex_lock(&player->mutex);
            double apts = PlayUtil::get()->apts;
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);

            pthread_mutex_lock(&player->mutex);

            g_isLock = true;

            //求一秒中平均一画帧/音帧播放要的间隔时间
            double audio_intervals = 1000.0 / PlayUtil::get()->audioFrameRate;
            double video_intervals = 1000.0 / PlayUtil::get()->videoFrameRate ;
            //帧倍率（表示一秒中播放音帧数是画帧数的几倍）
            double frame_magnification_rate = (double) PlayUtil::get()->audioFrameRate /(double) PlayUtil::get()->videoFrameRate;
//            LOGE("视频播放时间：%f   帧时：%f    %f",getCurrentTime(),apts ,d.pts);



            double diff = 0;
            if (PlayUtil::get()->apts != NULL_VALUE) {
                diff = d.pts  - PlayUtil::get()->apts;
                double allowable_error_time = (frame_magnification_rate * audio_intervals)/2.0;
//                LOGE("画帧时与音帧时的差值：%f   误差范围：%f",diff,allowable_error_time);
                if (diff <0){
                    diff = abs(diff);
                    if (diff <= allowable_error_time){
                        diff = 0;
                    } else {
                        double difference = diff - allowable_error_time;

                        if (difference > video_intervals){
                            av_frame_free(&player->ffVideoDecode->avFrame);
                            pthread_cond_broadcast(&player->cond);
                            pthread_mutex_unlock(&player->mutex);
                            return 0;//跳过这一帧
                        } else {
                            diff = -difference;
                        }
                    }

                } else if(diff > 0) {
                    if (diff <= allowable_error_time){
                        diff = 0;
                    } else {
                        double difference = diff - allowable_error_time;
                        diff = difference;
                    }
                }
            }

            if(PlayUtil::get()->videoOldPlayTime != -1){
                g_currentTime = getCurrentTime();
                double remaining_delay = g_currentTime + (video_intervals - (g_currentTime - PlayUtil::get()->videoOldPlayTime)) + diff;
                while (g_currentTime < remaining_delay){
                    if(!g_isLock){
                        pthread_mutex_lock(&player->mutex);
                        g_isLock = true;
                    }
                    g_currentTime = getCurrentTime();

                    if (g_currentTime < remaining_delay){
                        g_isLock = false;
                        pthread_cond_broadcast(&player->cond);
                        pthread_mutex_unlock(&player->mutex);
                    }
                }
            }

            PlayUtil::get()->videoOldPlayTime = getCurrentTime();
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);




            pthread_mutex_lock(&FFPlayer::Get()->mutex);
            player->videoView->render(d);
            pthread_cond_broadcast(&FFPlayer::Get()->cond);
            pthread_mutex_unlock(&FFPlayer::Get()->mutex);


            pthread_mutex_lock(&FFPlayer::Get()->mutex);
            av_frame_free(&player->ffVideoDecode->avFrame);
            pthread_cond_broadcast(&FFPlayer::Get()->cond);
            pthread_mutex_unlock(&FFPlayer::Get()->mutex);
        }
        return 0;
    }

    return -1;
}

void FFPlayer::playerCallBack(int thread_id,int status) {
    if (g_javaVM == nullptr){
        return;
    }

    // 将当前线程添加到 Java 虚拟机上
    JNIEnv *env = NULL;
    if( g_javaVM->AttachCurrentThread(&env, nullptr) == 0){
        if (g_callPlayStatus != nullptr) {
            //回调数据到Java函数上
            env->CallStaticVoidMethod(g_ffmpegJni,g_callPlayStatus,thread_id,status);
        }
        // 从 Java 虚拟机上分离当前线程
        g_javaVM->DetachCurrentThread();
    }
}

static void *runReadAVFormat(void *arg) {
    FFPlayer *player = (FFPlayer *) arg;

    restart:
    const char *url = PlayUtil::get()->playbackSource;
    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);


    LOGI("play url:%s", url);
    player->restartClear();
    bool is = player->ffDemux->open(url);
    if (is) {
        LOGI("解封装初始化成功 ");
        PlayUtil::get()->isRunFormat = true;
        /*
        * 注意 解码视频/音频 获取解码上下文（AVCodecContext） 都可以使用该方法，
        * 但音视频的 解码上下文目前是同一个，避免数据混乱使用需要区分
        * */
        Parameter videoPar = player->ffDemux->getVideoPara();
        player->ffVideoDecode->decoding(videoPar, false);

        Parameter audioPar = player->ffDemux->getAudioPara();
        player->ffAudioDecode->decoding(audioPar, false);
        player->ffResample->resample(audioPar);
        player->audioPlay->init(audioPar);


        LOGI(">>>视频初始化状态 %d   音频初始化状态 %d", videoPar.para->format, audioPar.para->format);

    } else {
        LOGI("解封装失败 ");
        PlayUtil::get()->isRunFormat = false;
    }

    while (PlayUtil::get()->isRunFormat) {

        if (PlayUtil::get()->switchPlaySource) {
            PlayUtil::get()->switchPlaySource = false;
            PlayUtil::get()->isPlayPause = true;
            PlayUtil::get()->isRunFormat = false;
            sleep(2);
            goto restart;
        }

        if (PlayUtil::get()->isPlayPause) {
            PlayUtil::get()->playPauseSleep();
            continue;
        }

        Data data = player->ffDemux->readAVFormatData();
        if (!data.data || data.size == 0) {
            PlayUtil::get()->noDataSleep();
            continue;
        }

        pthread_mutex_lock(&player->mutex);
        if (data.data) {
            if (data.stream_index == STREAM_VIDEO) {
                player->decodeVideoList.push_back(data);
            } else if (data.stream_index == STREAM_AUDIO) {
                player->decodeAudioList.push_back(data);
            }
//            LOGE(">>>>>>>>>>>>>>>>> 解封装中 ");
        }
        pthread_cond_broadcast(&player->cond);
        pthread_mutex_unlock(&player->mutex);
    }
    LOGI(">>>>>>>>>>>>>>>>> 退出解封装 ");

    end:
    PlayUtil::get()->isRunFormat = false;

//    pthread_exit(&player->avFormatPt);

    return 0;
}

static void *runDecode(void *arg) {
    FFPlayer *player = (FFPlayer *)arg;
    PlayUtil::get()->isRunPlayVideo = true;
    while (PlayUtil::get()->isRunPlayVideo) {
//        LOGE("缓冲数据满了  %d ",player->audioPlay->isPlayAudio);
        if(PlayUtil::get()->isPlayPause){
            PlayUtil::get()->playPauseSleep();
            continue;
        }

        pthread_mutex_lock(&player->mutex);

        if (!PlayUtil::get()->canPlayAudio){
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);
            continue;
        }


        if (player->decodeVideoList.empty()){
//            LOGE("缓冲数据用完了");
            PlayUtil::get()->canPlayVideo = false;
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);
            PlayUtil::get()->noDataSleep();
            continue;
        }

        Data data = player->decodeVideoList.front();
        PlayUtil::get()->vpts = data.pts;
//        LOGE("1视频播放时间：%f   帧时：%f    %f",getCurrentTime(), GlobalState::State()->apts,GlobalState::State()->vpts);
        pthread_cond_broadcast(&player->cond);
        pthread_mutex_unlock(&player->mutex);

        player->decode_video(player, &data);//播放视频

        pthread_mutex_lock(&player->mutex);
        if (data.data) {
            av_packet_unref((AVPacket *) data.data);
        }
        data.Drop();
        player->decodeVideoList.pop_front();
        pthread_cond_broadcast(&player->cond);
        pthread_mutex_unlock(&player->mutex);

    }
    PlayUtil::get()->isRunPlayVideo = false;
//    pthread_exit(&FFPlayer::Get()->avCodecPt);
    return 0;
}

static void *runDecode2(void *arg) {
    FFPlayer *player = (FFPlayer *)arg;
    PlayUtil::get()->isRunPlayVideo = true;
    while (PlayUtil::get()->isRunPlayVideo) {

        if(PlayUtil::get()->isPlayPause){
            PlayUtil::get()->playPauseSleep();
            continue;
        }

        pthread_mutex_lock(&player->mutex);
        if (player->decodeAudioList.empty()){
            pthread_cond_broadcast(&player->cond);
            pthread_mutex_unlock(&player->mutex);
            PlayUtil::get()->noDataSleep();
            continue;
        }

        Data data = player->decodeAudioList.front();

        pthread_cond_broadcast(&player->cond);
        pthread_mutex_unlock(&player->mutex);

        player->decode_audio(player,&data);//播放音频

        pthread_mutex_lock(&player->mutex);
        if (data.data) {
            av_packet_unref((AVPacket *) data.data);
        }
        data.Drop();
        player->decodeAudioList.pop_front();
        pthread_cond_broadcast(&player->cond);
        pthread_mutex_unlock(&player->mutex);

    }
    PlayUtil::get()->isRunPlayVideo = false;
//    pthread_exit(&FFPlayer::Get()->avCodecPt);
    return 0;
}





extern "C"
JNIEXPORT void JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_initView(JNIEnv *env, jclass thiz, jobject surface) {
    g_ffmpegJni = thiz;
    g_win = ANativeWindow_fromSurface(env, surface);
    FFPlayer::Get()->videoView->setRender(g_win);

    /*
     * 通过JNIEnv找到FFmpegJNI该类，则只能通过Java方法传过来的JNIEnv；
     * 将该回调方法置为全局，以便在c/c++ 线程中调用
     * */
    static const char *const DL_CLASS_NAME = "com/video/ffmpeg/jniapi/FFmpegJNI";
    jclass ffmpegJNI = env->FindClass(DL_CLASS_NAME);
    if (ffmpegJNI != nullptr) {
        g_callPlayStatus = env->GetStaticMethodID(ffmpegJNI, "playerCallBack", "(II)V");
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_play(JNIEnv *env, jclass clazz, jstring url) {

    FFPlayer *player = FFPlayer::Get();
    const char *c_url = env->GetStringUTFChars(url, 0);
    if (PlayUtil::get()->isRunFormat && PlayUtil::get()->playbackSource){
        delete PlayUtil::get()->playbackSource;
        PlayUtil::get()->playbackSource = 0;
        PlayUtil::get()->playbackSource = c_url;
        PlayUtil::get()->switchPlaySource = true;
        env->ReleaseStringUTFChars(url, NULL);
        return;
    }
    PlayUtil::get()->playbackSource = c_url;


    if (!PlayUtil::get()->isRunPlayVideo) {
        pthread_create(&player->videoPlayPt, NULL, runDecode, (void *) player);
    }
    if (!PlayUtil::get()->isRunPlayVideo) {
        pthread_create(&player->audioPlayPt, NULL, runDecode2, (void *) player);
    }

    if (!PlayUtil::get()->isRunFormat) {
        pthread_mutex_init(&player->mutex, NULL);
        pthread_cond_init(&player->cond, NULL);
        pthread_create(&player->avFormatPt, NULL, runReadAVFormat, (void *) player);
    }


    if (!PlayUtil::get()->isRunPlayVideo) {
        pthread_detach(player->videoPlayPt);
    }
    if (!PlayUtil::get()->isRunPlayVideo) {
        pthread_detach(player->audioPlayPt);
    }
    if (!PlayUtil::get()->isRunFormat) {
        pthread_detach(player->avFormatPt);
    }

    env->ReleaseStringUTFChars(url, NULL);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_pause(JNIEnv *env, jclass clazz, jboolean pause){
    PlayUtil::get()->isPlayPause = pause == JNI_TRUE;
}

extern "C"
JNIEXPORT double JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_totalTime(JNIEnv *env, jclass clazz) {
    return PlayUtil::get()->totalTime;
}

extern "C"
JNIEXPORT double JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_playbackProgress(JNIEnv *env, jclass clazz){
    if (PlayUtil::get()->totalTime > 0){
        return PlayUtil::get()->apts/PlayUtil::get()->totalTime;
    }
    return 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_seek(JNIEnv *env, jclass clazz, jdouble seek) {
    FFPlayer *player = FFPlayer::Get();
    if (PlayUtil::get()->totalTime > 0){
        PlayUtil::get()->isPlayPause = true;
        player->audioPlay->clearResample();
        player->clearPacket();
        player->ffVideoDecode->clearAVCodec();
        player->ffAudioDecode->clearAVCodec();
        int re = player->ffDemux->seek(seek);
        PlayUtil::get()->isPlayPause = false;
        return re == 0; //等于零为成功
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_stop(JNIEnv *env, jclass clazz){
    PlayUtil::get()->isPlayPause = true;
    PlayUtil::get()->isRunFormat = false;
    PlayUtil::get()->isRunPlayVideo = false;

    FFPlayer *player = FFPlayer::Get();
    player->audioPlay->clearResample();
    player->clearPacket();
    player->ffVideoDecode->clearAVCodec();
    player->ffAudioDecode->clearAVCodec();

    player->videoView->close();
    player->audioPlay->close();
    player->ffVideoDecode->closeAVCodec();
    player-> ffAudioDecode->closeAVCodec();
    player->ffDemux->close();

    //释放窗口
    if (g_win) {
        ANativeWindow_release(g_win);
    }

    PlayUtil::get()->isPlayPause = false;//还原
}



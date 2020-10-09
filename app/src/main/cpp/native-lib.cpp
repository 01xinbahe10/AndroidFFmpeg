#include <jni.h>
#include <string>
#include "FFPlayer.h"
#include "Log.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_video_ffmpeg_jniapi_FFmpegJNI_stringFromJNI(JNIEnv *env, jclass clazz) {
    // TODO: implement stringFromJNI()
    std::string hello = "Hello from C++";
//    hello += avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}


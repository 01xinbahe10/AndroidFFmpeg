package com.video.ffmpeg.jniapi;

import android.util.Log;

/**
 * Created by hxb on  2020/3/5
 */
public final class FFmpegJNI {
    private static final String TAG = "FFmpegJNI";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public static native String stringFromJNI();

    public static native void initView(Object surface);

    public static native void play(String url);

    public static native void pause(boolean pause);

    //播放总时长
    public static native double totalTime();

    //播放进度的百分比，如是直播则无百分比
    public static native double playbackProgress();

    //设置播放位置
    public static native boolean seek(double seek);

    public static native void stop();

    private static void playerCallBack(int thread_id,int status){
        // TODO: 2020/7/30 禁止在该回调方法中做任何延时或耗时操作
        Log.e(TAG, "callPlayStatus: "+thread_id+"    >>>>  "+status );
    }
}

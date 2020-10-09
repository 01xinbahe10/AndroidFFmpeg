package com.video.ffmpeg.jniapi;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by hxb on  2020/6/16
 */
public class FFPlayer extends GLSurfaceView implements SurfaceHolder.Callback,GLSurfaceView.Renderer{
//    public FFPlayer(Context context) {
//        super(context);
//        init();
//    }

    public FFPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        /*
        * 小知识点：
        * 这里如果不想将Surface交给系统使用，
        * 想自己使用，请注释掉 super.surfaceCreated(holder); 这一句。
        * */
//        super.surfaceCreated(holder);
        FFmpegJNI.initView(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        super.surfaceChanged(holder, format, w, h);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }

    private void init(){
        setRenderer( this );
    }
}

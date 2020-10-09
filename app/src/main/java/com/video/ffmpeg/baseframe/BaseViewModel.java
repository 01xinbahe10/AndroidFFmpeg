package com.video.ffmpeg.baseframe;

import android.app.Application;
import android.os.Handler;
import android.os.Message;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleOwner;

/**
 * Created by hxb on 2019-08-23.
 *  BaseViewModel：属于 MVVM  中的VM层,主要作用是跟view层和model层做交互及业务逻辑实现
 *
 */
public class BaseViewModel<R extends BaseRepository> extends AndroidViewModel implements IBaseViewModel {

    //数据管理类
    protected R repository;
    public Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {

            return handlerCallBack(msg);
        }
    });
    public BaseViewModel(@NonNull Application application) {
        super(application);
    }

    public BaseViewModel(@NonNull Application application, R repository){
        super(application);
        this.repository = repository;
    }
    @Override
    public void onAny(LifecycleOwner owner, Lifecycle.Event event) {

    }

    @Override
    public void onCreate() {

    }

    @Override
    public void onStart() {

    }

    @Override
    public void onStop() {

    }

    @Override
    public void onResume() {

    }

    @Override
    public void onPause() {

    }

    @Override
    public void onDestroy() {
        if (null != mHandler){
            mHandler.removeCallbacksAndMessages(null);
        }
    }

    @Override
    protected void onCleared() {
        super.onCleared();
        if (null != repository) {
            repository.onCleared();
        }
    }

    public boolean handlerCallBack(Message msg){

        return false;
    }
}

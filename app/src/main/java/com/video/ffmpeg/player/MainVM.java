package com.video.ffmpeg.player;

import android.app.Application;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.databinding.ObservableField;

import com.video.ffmpeg.baseframe.BaseViewModel;
import com.video.ffmpeg.jniapi.FFmpegJNI;

/**
 * Created by hxb on  2020/3/5
 */
public class MainVM extends BaseViewModel {
    private final String TAG = "MainVM";
    public ObservableField<String> textHello = new ObservableField<>("");
    public MainVM(@NonNull Application application) {
        super(application);
        String text = FFmpegJNI.stringFromJNI();
        textHello.set(text);
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onResume() {
        super.onResume();

    }


}

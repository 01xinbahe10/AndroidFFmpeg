package com.video.ffmpeg.player;

import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import androidx.core.app.ActivityCompat;
import androidx.lifecycle.Observer;

import com.video.ffmpeg.BR;
import com.video.ffmpeg.R;
import com.video.ffmpeg.baseframe.BaseActivity;
import com.video.ffmpeg.baseframe.util.LiveDataBus;
import com.video.ffmpeg.baseframe.util.PermissionUtils;
import com.video.ffmpeg.data.vo.ResVO;
import com.video.ffmpeg.databinding.ActivityMainBinding;
import com.video.ffmpeg.jniapi.FFmpegJNI;
import com.video.ffmpeg.player.dlg.AddResDlg;
import com.video.ffmpeg.player.dlg.ResListDlg;

public class MainActivity extends BaseActivity<ActivityMainBinding, MainVM> implements View.OnClickListener {
    private final String TAG = "MainActivity";

    @Override
    public void initParam() {
        configWindow();
    }

    @Override
    protected int initContentView(Bundle savedInstanceState) {
        return R.layout.activity_main;
    }

    @Override
    protected int initViewModelId() {
        return BR._all;
    }

    @Override
    public void initData() {
        requestPower();
    }

    @Override
    public void initViewObservable() {
        super.initViewObservable();
        subData();
        viewDataBinding.ivAdd.setOnClickListener(this);
        viewDataBinding.ivList.setOnClickListener(this);
        viewDataBinding.ivPause.setOnClickListener(this);
        viewDataBinding.ivFastRewind.setOnClickListener(this);
        viewDataBinding.ivFastForward.setOnClickListener(this);

//        MediaPlayer
    }


    @Override
    protected void onResume() {
        super.onResume();
//        viewDataBinding.sampleText.setText(baseViewModel.textHello.get());
    }

    @Override
    protected void onDestroy() {
        AddResDlg.instance(this).destroy();
        ResListDlg.instance(this).destroy();
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.iv_add:
                AddResDlg.instance(this).show();
                break;
            case R.id.iv_list:
                ResListDlg.instance(this).show();
                break;
            case R.id.iv_pause:
                if (viewDataBinding.ivPause.getDrawable().getLevel() == 0){
                    FFmpegJNI.pause(true);
                    viewDataBinding.ivPause.getDrawable().setLevel(1);
                }else {
                    FFmpegJNI.pause(false);
                    viewDataBinding.ivPause.getDrawable().setLevel(0);
                }
                break;
            case R.id.iv_fast_rewind:
                break;
            case R.id.iv_fast_forward:
                FFmpegJNI.seek(0.5);
                break;
        }
    }

    /*
    * 设置屏幕
    * */
    private void configWindow(){
        //去掉标题栏
        supportRequestWindowFeature( Window.FEATURE_NO_TITLE);
        //全屏，隐藏状态
        getWindow().setFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN ,
                WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
        //屏幕保持常亮
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    }

    /*
     * 请求权限
     */
    public void requestPower() {
        if (PermissionUtils.checkStorage(this) != PermissionUtils.SUCCESS) {
            ActivityCompat.requestPermissions(this,
                    PermissionUtils.STORAGE_PERMISSIONS, 1);
        }
    }

    private void subData(){
        LiveDataBus.get().with(G.MainA.SUB_DATA,Object[].class).observe(this, new Observer<Object[]>() {
            @Override
            public void onChanged(Object[] objects) {
                if (null == objects || objects.length != 2){
                    return;
                }
                String tag = (String) objects[0];
                Object value = objects[1];
                switch (tag){
                    case G.MainA.tClickItemRes:
                        final ResVO resVO = (ResVO) value;
//                        Log.e(TAG, "onChanged: PPPPPPPPPPPPPPPPPPPPPPP  "+resVO.getResAddress() );
//
//                        new Thread(new Runnable() {
//                            @Override
//                            public void run() {
//                                FFmpegJNI.clear();
                                FFmpegJNI.play(resVO.getResAddress());

//                            }
//                        }).start();
//
                        break;
                }
            }
        });
    }


}

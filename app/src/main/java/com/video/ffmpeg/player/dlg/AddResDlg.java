package com.video.ffmpeg.player.dlg;

import android.content.Context;
import android.os.Handler;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;

import com.video.ffmpeg.R;
import com.video.ffmpeg.baseframe.BaseApplication;
import com.video.ffmpeg.data.db_control.ResDBControl;

/**
 * Created by hxb on  2020/6/11
 */
public class AddResDlg implements View.OnClickListener {
    private static AddResDlg mDlg = null;
    private final String TAG = "AddResDlg";

    private Context context;
    private Handler handler = new Handler();
    private AlertDialog dialog;
    private View rootView;

    public static AddResDlg instance(Context context) {
        if (null == mDlg) {
            mDlg = new AddResDlg();
        }
        mDlg.context = context;
        return mDlg;
    }

    private AddResDlg() {
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.iv_close:
                if (null != dialog) {
                    dialog.dismiss();
                }
                break;
            case R.id.bt_ok:
                String name = null != etName ? etName.getText().toString().trim():null;
                String address = null != etAddress?etAddress.getText().toString().trim():null;
                if (TextUtils.isEmpty(name) || TextUtils.isEmpty(address)){
                    Toast.makeText(context,"任何一个都不能为空！",Toast.LENGTH_SHORT).show();
                    break;
                }
                ResDBControl dbControl = ResDBControl.getDB(BaseApplication.getInstance());
                if (dbControl.saveOneRes(name,address)){
                    Toast.makeText(context,"保存成功",Toast.LENGTH_SHORT).show();
                }else {
                    Toast.makeText(context,"保存失败，再试一次！",Toast.LENGTH_SHORT).show();
                }
                break;
        }
    }

    private AlertDialog initDlg(Context context, View view) {
        AlertDialog alertDialog = new AlertDialog.Builder(context, R.style.AlertDialog)
                .setCancelable(false)
                .setView(view)
                .show();
        Window window = alertDialog.getWindow();
        WindowManager.LayoutParams params =
                window.getAttributes();
        params.width = ViewGroup.LayoutParams.MATCH_PARENT;
        params.height = ViewGroup.LayoutParams.WRAP_CONTENT;
        window.setAttributes(params);
        return alertDialog;
    }

    //初始化控件
    private ImageView ivClose;
    private Button btOk;
    private EditText etName,etAddress;
    private void initControls(View view) {
        if (null == ivClose) {
            ivClose = view.findViewById(R.id.iv_close);
            ivClose.setOnClickListener(this);
        }
        if (null == btOk) {
            btOk = view.findViewById(R.id.bt_ok);
            btOk.setOnClickListener(this);
        }
        if (null == etName) {
            etName = view.findViewById(R.id.et_name);
        }
        if (null == etAddress) {
            etAddress = view.findViewById(R.id.et_address);
        }
    }

    //显示弹窗
    public void show() {
        if (null != dialog && dialog.isShowing()) {
            return;
        }
        if (null == rootView) {
           new Thread(new Runnable() {
               @Override
               public void run() {
                   rootView = LayoutInflater.from(context).inflate(R.layout.dlg_add_res, null, false);
                   handler.post(new Runnable() {
                       @Override
                       public void run() {
                           dialog = initDlg(context, rootView);
                           initControls(rootView);
                       }
                   });
               }
           }).start();
            return;
        }
        if (null != dialog) {
            dialog.cancel();
            dialog.show();
            initControls(rootView);
            return;
        }
        dialog = initDlg(context, rootView);
        initControls(rootView);
    }

    public void destroy() {
        if (null != dialog) {
            dialog.dismiss();
        }
        mDlg = null;
    }

}

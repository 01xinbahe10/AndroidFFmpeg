package com.video.ffmpeg.baseframe.util;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.PermissionChecker;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;


/**
 * Created by hxb on 2018/7/16.
 * 该类主要做Android手机中的权限处理
 */

public final class PermissionUtils {

    //权限申请状态返回码
    public static final int SUCCESS = 0;
    public static final int FAIL = -1;
    public static final int APPLIED_AND_REJECTED = 1;//表示之前申请过但用户拒绝了

    //可以将Android6.0以上的需要动态申请的权限，在这里集中一起
    public static final String[] ALL_PERMISSIONS = new String[]{};

    //相机权限
    public static final String[] CAMERA_PERMISSIONS = {Manifest.permission.CAMERA};

    //定位权限
    public static final String[] POSITION_PERMISSIONS = {Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION};

    //储存权限
    public static final String[] STORAGE_PERMISSIONS = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};

    //录制权限
    public static final String[] RECORD_PERMISSIONS  = {Manifest.permission.RECORD_AUDIO};

    //默认返回码
    public static int defRequestCode() {
        return 10_000;
    }



    /**
     * 检查定位权限是否开启
     *
     * @return false:表示权限不通过
     */
    @SuppressLint("WrongConstant")
    public static boolean checkPosition(Context context, int version) {

        //如果 API level 是大于等于 27(Android 8.0) 时
        //没有开启该权限，就请求该权限
        if (Build.VERSION.SDK_INT >= version) {
            return PermissionChecker.checkSelfPermission(context, POSITION_PERMISSIONS[0]) == PackageManager.PERMISSION_GRANTED &&
                    PermissionChecker.checkSelfPermission(context, POSITION_PERMISSIONS[1]) == PackageManager.PERMISSION_GRANTED;
        }
        return true;

    }

    /**
     * 检查权限
     * */

    public static int checkPermissions(Activity activity, String[] permissions){
        for (String permission:permissions) {
            if (ContextCompat.checkSelfPermission(activity, permission) != PERMISSION_GRANTED) {
                if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
                    return APPLIED_AND_REJECTED;
                }
                return FAIL;
            }

        }
        return SUCCESS;
    }


    /**
     * 检查存储权限
     * */
    public static int checkStorage(Activity activity){
        for (String permission:STORAGE_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(activity, permission) != PERMISSION_GRANTED) {
                if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
                    return APPLIED_AND_REJECTED;
                }
                return FAIL;
            }

        }
        return SUCCESS;
    }

    /**
     * 检查录制权限
     */
    public static int checkRecord(Activity activity) {
        for (String permission:RECORD_PERMISSIONS) {
            //判断是否已经赋予权限 WRITE_EXTERNAL_STORAGE
            if (ContextCompat.checkSelfPermission(activity, permission) != PERMISSION_GRANTED) {
                //如果应用之前请求过此权限但用户拒绝了请求，此方法将返回 true。
                if (ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)) {
                    //这里可以写个对话框之类的项向用户解释为什么要申请权限，并在对话框的确认键后续再次申请权限 RECORD_AUDIO
                    //权限是被拒绝的
                    return APPLIED_AND_REJECTED;
                }
                return FAIL;
            }

        }
        return SUCCESS;
    }




}



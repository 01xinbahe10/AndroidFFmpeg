package com.video.ffmpeg.player.dlg;

import android.content.Context;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.databinding.ViewDataBinding;
import androidx.lifecycle.LiveData;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.video.ffmpeg.R;
import com.video.ffmpeg.baseframe.BaseApplication;
import com.video.ffmpeg.baseframe.adapter.BaseRecycler1Adapter;
import com.video.ffmpeg.baseframe.util.LiveDataBus;
import com.video.ffmpeg.data.db_control.ResDBControl;
import com.video.ffmpeg.data.vo.ResVO;
import com.video.ffmpeg.databinding.ItemResBinding;
import com.video.ffmpeg.player.G;

import java.util.List;

/**
 * Created by hxb on  2020/6/11
 */
public class ResListDlg implements View.OnClickListener {

    private static ResListDlg mDlg = null;
    private final String TAG = "AddResDlg";

    private Context context;
    private Handler handler = new Handler();
    private AlertDialog dialog;
    private View rootView;

    public static ResListDlg instance(Context context) {
        if (null == mDlg) {
            mDlg = new ResListDlg();
        }
        mDlg.context = context;
        return mDlg;
    }

    private ResListDlg() {
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.iv_close:
                if (null != dialog) {
                    dialog.dismiss();
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
    private RecyclerView rvRes;
    private TextView tvNoDataHint;
    private Adapter adapter;
    private void initControls(View view) {
        if (null == ivClose) {
            ivClose = view.findViewById(R.id.iv_close);
            ivClose.setOnClickListener(this);
        }

        if (null == rvRes) {
            rvRes = view.findViewById(R.id.rv_res);
            rvRes.setLayoutManager(new LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false));
        }

        if (null == tvNoDataHint){
            tvNoDataHint = view.findViewById(R.id.tv_no_data_hint);
        }

        if (null == adapter) {
            adapter = new Adapter(context);
        }

        rvRes.setAdapter(adapter);

        new Thread(new Runnable() {
            @Override
            public void run() {
                ResDBControl dbControl = ResDBControl.getDB(BaseApplication.getInstance());
                final List<ResVO> list = dbControl.readAllRes();
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        boolean isResNull = null == list || list.isEmpty();
                        tvNoDataHint.setVisibility(isResNull?View.VISIBLE:View.GONE);

                        if (!adapter.getItemsVO().isEmpty()) {
                            adapter.getItemsVO().clear();
                        }
                        if (!isResNull) {
                            adapter.getItemsVO().addAll(list);
                        }
                        adapter.notifyDataSetChanged();
                    }
                });
            }
        }).start();

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
                    rootView = LayoutInflater.from(context).inflate(R.layout.dlg_res_list, null, false);
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

    private class Adapter extends BaseRecycler1Adapter<ItemResBinding, ResVO> {
        public Adapter(Context context) {
            super(context);
        }

        @Override
        protected int getLayoutResId(int viewType) {
            return R.layout.item_res;
        }

        @Override
        protected int viewModelId() {
            return 0;
        }

        @Override
        protected void viewInstance(final ItemResBinding viewBinding) {
            super.viewInstance(viewBinding);
            viewBinding.clResParent.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int position = recyclerView.getChildLayoutPosition(viewBinding.getRoot());
                    LiveDataBus.get().with(G.MainA.SUB_DATA).setValue(new Object[]{G.MainA.tClickItemRes, getItemsVO().get(position)});
                    if (null != dialog){
                        dialog.dismiss();
                    }
                }
            });

            viewBinding.ivDel.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    int position = recyclerView.getChildLayoutPosition(viewBinding.getRoot());
                    ResDBControl resDBControl = ResDBControl.getDB(BaseApplication.getInstance());
                    ResVO resVO = getItemsVO().get(position);
                    if (resDBControl.delOneRes(resVO.getName())){
                        Toast.makeText(context,"删除成功！",Toast.LENGTH_SHORT).show();
                        initControls(rootView);
                    }else {
                        Toast.makeText(context,"删除失败，请再试一次！",Toast.LENGTH_SHORT).show();
                    }
                }
            });
        }

        @Override
        protected void onBindItem(ItemResBinding viewBinding, ResVO itemVO, int position) {
            viewBinding.tvName.setText(("名字：" + itemVO.getName()));
            viewBinding.tvAddress.setText(("播放地址：" + itemVO.getResAddress()));
        }
    }
}

package com.video.ffmpeg.data.db_control;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import com.video.ffmpeg.baseframe.db.ISQLiteHelper;
import com.video.ffmpeg.baseframe.db.SQLiteAssetHelper;
import com.video.ffmpeg.data.vo.ResVO;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by hxb on  2020/6/11
 */
public class ResDBControl implements IDBControl {
    private static ResDBControl mControl = null;
    private ISQLiteHelper mIHelper = null;
    private String TABLE_NAME = "";
    private Cursor mCursor = null;

    /*版本1的字段*/
    interface DBVersion1 {
        int version = 1;//版本号
        /*表名与字段名*/
        String tableName = "filmRes";
        String f_name = "name";
        String f_address = "address";
    }

    public static ResDBControl getDB(final Context context) {
        if (null != mControl) {
            return mControl;
        }
        mControl = new ResDBControl();
        mControl.TABLE_NAME = DBVersion1.tableName;
        mControl.mIHelper = SQLiteAssetHelper.init(context, new ISQLiteHelper.SQLiteInit() {
            @Override
            public String databaseName() {
                return DB_NAME;
            }

            @Override
            public int databaseVersion() {
                return DBVersion1.version;
            }

            @Override
            public void databaseCreate(SQLiteDatabase db) {
                mControl.createTable(db);
            }

            @Override
            public void databaseUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

            }
        });
        return mControl;
    }

    private ResDBControl() {
    }

    /*
     * 建表
     * */
    private void createTable(SQLiteDatabase db) {
//        String tableField = ","+DBVersion1.f_key+" char ,"+DBVersion1.f_binaryValue+" blob";
//        String sql = "create table if not exists " + tableName + " (_id integer primary key autoincrement" + tableField + ")";

        String tableField = DBVersion1.f_name + " varchar(255) primary key not null," +DBVersion1.f_address+" varchar(255)";
        String sql = "create table if not exists " + TABLE_NAME + " (" + tableField + ")";
        db.execSQL(sql);
    }


    /*
     * 保存一条数据
     * */
    public boolean saveOneRes( final String name, final String address){
        final boolean[] is = {false};
        mIHelper.carriedOutWritable(new ISQLiteHelper.SQLiteDataAction() {
            @Override
            public void action(SQLiteDatabase db) {
                createTable(db);//如不存在就建表

                ContentValues contentValues2 = null;
                try {
                    contentValues2 = new ContentValues();
                    contentValues2.put(DBVersion1.f_name, name);
                    contentValues2.put(DBVersion1.f_address, address);
                } catch (Exception e) {
                    e.printStackTrace();
                }

                if (null == contentValues2) {
                    is[0] = false;
                } else {
                    long i = db.replace(TABLE_NAME, null, contentValues2);
//                    Log.e(TAG, "error: ppppppppppppppppppppp   ttt " + i+"  "+TABLE_NAME);
                    is[0] = true;
                }
            }

            @Override
            public void actionFinally() {

            }

            @Override
            public void error(Exception e) {
                is[0] = false;
            }
        });

        return is[0];
    }

    /*
     * 删除一条数据
     * */
    public boolean delOneRes(final String name){
        final boolean[] is = {false};
        mIHelper.carriedOutWritable(new ISQLiteHelper.SQLiteDataAction() {
            @Override
            public void action(SQLiteDatabase db) {
                db.delete(TABLE_NAME, (DBVersion1.f_name + " = ?"), new String[]{name});
                is[0] = true;
            }

            @Override
            public void actionFinally() {

            }

            @Override
            public void error(Exception e) {
                is[0] = false;
            }
        });
        return is[0];
    }

    /*
     * 读取全部数据
     * */
    public List<ResVO> readAllRes(){
        final List<ResVO>[] objects = new List[]{null};
        mIHelper.carriedOutReadable(new ISQLiteHelper.SQLiteDataAction() {
            @Override
            public void action(SQLiteDatabase db) {
                List<ResVO> list = new ArrayList<>();
                mCursor = db.query(TABLE_NAME, null, null, null, null, null,null);
                if (mCursor.moveToFirst()){
                    do{
                        String name = mCursor.getString(mCursor.getColumnIndex(DBVersion1.f_name));
                        String address = mCursor.getString(mCursor.getColumnIndex(DBVersion1.f_address));
                        ResVO devVO = ResVO.init().setName(name).setResAddress(address);
                        Log.e("TAG", "action: "+devVO.toString() );
                        list.add(devVO);

                    }while (mCursor.moveToNext());
                }
                objects[0] = list;
            }

            @Override
            public void actionFinally() {

            }

            @Override
            public void error(Exception e) {
                objects[0] = null;
            }
        });
        return objects[0];
    }

}

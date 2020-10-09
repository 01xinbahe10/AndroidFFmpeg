package com.video.ffmpeg.baseframe.util;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Arrays;

/**
 * Created by hxb on  2020/2/25
 * 数据工具(处理文件，对象)
 */
public class DataUtil {

    /**
     * 克隆对象(前提是需要 类型 需要序列化)
     * */
    public static <T> T cloneObject(T obj) {
        T result = null;
        ByteArrayOutputStream byteArrayOutputStream = null;
        ByteArrayInputStream byteArrayInputStream = null;
        ObjectOutputStream outputStream = null;
        ObjectInputStream inputStream = null;
        try {
            //对象写到内存中
            byteArrayOutputStream = new ByteArrayOutputStream();
            outputStream = new ObjectOutputStream(byteArrayOutputStream);
            outputStream.writeObject(obj);

            //从内存中再读出来
            byteArrayInputStream = new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
            inputStream = new ObjectInputStream(byteArrayInputStream);
            result = (T) inputStream.readObject();

        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } finally {
            try {
                if (outputStream != null)
                    outputStream.close();
                if (inputStream != null)
                    inputStream.close();
                if (byteArrayOutputStream != null)
                    byteArrayOutputStream.close();
                if (byteArrayInputStream != null)
                    byteArrayInputStream.close();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return result;
    }


    /**
     * 数组合并
     * */
    public static <T> Object[] arrayMerge(T[] firstArray, T[] twoArray) {
        int firstLength = firstArray.length;
        int twoLength = twoArray.length;
        T[] newArray = Arrays.copyOf(firstArray, (firstLength + twoLength));
        System.arraycopy(twoArray, 0, newArray, firstLength, twoLength);
        return newArray;
    }
}

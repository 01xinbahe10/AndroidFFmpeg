package com.video.ffmpeg.data.vo;

/**
 * Created by hxb on  2020/6/11
 */
public class ResVO {
    private String name = "";
    private String resAddress = "";
    private ResVO(){}

    public static ResVO init(){
        return new ResVO();
    }

    public ResVO setName (String name){
        this.name = name;
        return this;
    }

    public ResVO setResAddress(String resAddress){
        this.resAddress = resAddress;
        return this;
    }

    public String getName() {
        return name;
    }

    public String getResAddress() {
        return resAddress;
    }
}

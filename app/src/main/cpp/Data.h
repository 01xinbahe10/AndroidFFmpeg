
//
// Created by Administrator on 2018-03-01.
//

#ifndef XPLAY_XDATA_H
#define XPLAY_XDATA_H
enum XDataType
{
    AVPACKET_TYPE = 0,
    UCHAR_TYPE = 1,

    STREAM_NO = 0,
    STREAM_VIDEO =1,
    STREAM_AUDIO = 2


};


struct Data
{
    int type = 0;
    double pts = 0;
    unsigned char *data = 0;
    unsigned char *datas[8] = {0};
    int size = 0;
    int stream_index = STREAM_NO; //0为无状态 1为视频 2为音频
    int width = 0;
    int height = 0;
    int format = 0;
    bool Alloc(int size,const char *data=0);
    void Drop();
};


#endif //XPLAY_XDATA_H

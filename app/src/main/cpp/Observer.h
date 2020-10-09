//
// Created by xb on 2020/7/28.
//

#ifndef MINEFFMPEG_OBSERVER_H
#define MINEFFMPEG_OBSERVER_H

class Observer{
public:
    //播放器回调状态
    virtual void playerCallBack(int thread_id,int status) {};
};

#endif //MINEFFMPEG_OBSERVER_H

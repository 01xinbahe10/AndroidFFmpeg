//
// Created by xb on 2020/6/10.
//

#ifndef MINEFFMPEG_VIDEOVIEW_H
#define MINEFFMPEG_VIDEOVIEW_H

#include "Data.h"
#include "Texture.h"

class Texture;
class VideoView{
public:
    virtual void setRender(void *win);
    virtual void render(Data data);
    virtual void close();
protected:
    void *view = 0;
    Texture *txt = 0;
};
#endif //MINEFFMPEG_VIDEOVIEW_H

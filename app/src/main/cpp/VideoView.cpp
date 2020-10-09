//
// Created by xb on 2020/6/10.
//

#include "VideoView.h"
#include "Texture.h"
#include "Log.h"

void VideoView::setRender(void *win) {
    close();
    view = win;
}

void VideoView::close() {
    if (txt) {
        txt->Drop();
        txt = 0;
    }

}

void VideoView::render(Data data) {
    if (!view) return;
    if (!txt) {
        txt = Texture::Create();
        bool is = txt->Init(view, (TextureType) data.format);
    }
    txt->Draw(data.datas, data.width, data.height);
}
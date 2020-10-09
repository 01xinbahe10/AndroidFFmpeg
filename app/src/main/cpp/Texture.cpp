/*******************************************************************************
**                                                                            **
**                     Jiedi(China nanjing)Ltd.                               **
**	               创建：夏曹俊，此代码可用作为学习参考                       **
*******************************************************************************/

/*****************************FILE INFOMATION***********************************
**
** Project       : FFmpeg
** Description   : FFMPEG项目创建示例
** Contact       : xiacaojun@qq.com
**        博客   : http://blog.csdn.net/jiedichina
**		视频课程 : 网易云课堂	http://study.163.com/u/xiacaojun		
				   腾讯课堂		https://jiedi.ke.qq.com/				
				   csdn学院		http://edu.csdn.net/lecturer/lecturer_detail?lecturer_id=961	
**                 51cto学院	http://edu.51cto.com/lecturer/index/user_id-12016059.html	
** 				   下载最新的ffmpeg版本 ffmpeg.club
**                 
**   安卓流媒体播放器 课程群 ：23304930 加入群下载代码和交流
**   微信公众号  : jiedi2007
**		头条号	 : 夏曹俊
**
*******************************************************************************/
//！！！！！！！！！ 加群23304930下载代码和交流


//
// Created by Administrator on 2018-03-04.
//

#include "Texture.h"
#include "Log.h"
#include "EGL.h"
#include "Shader.h"

class CXTexture : public Texture {
public:
    Shader sh;
    TextureType type;

    virtual void Drop() {
        EGL::Get()->Close();
        sh.Close();
        delete this;
    }

    virtual bool Init(void *win, TextureType type) {
        EGL::Get()->Close();
        sh.Close();
        this->type = type;
        if (!win) {
            LOGE("Texture Init failed win is NULL");
            return false;
        }
        if (!EGL::Get()->Init(win)) {
            return false;
        }
        sh.Init((XShaderType) type);
        return true;
    }

    virtual void Draw(unsigned char *data[], int width, int height) {
        sh.GetTexture(0, width, height, data[0]);  // Y

        if (type == XTEXTURE_YUV420P) {
            sh.GetTexture(1, width / 2, height / 2, data[1]);  // U
            sh.GetTexture(2, width / 2, height / 2, data[2]);  // V
        } else {
            sh.GetTexture(1, width / 2, height / 2, data[1], true);  // UV
        }
        sh.Draw();
        EGL::Get()->Draw();
    }

};

Texture *Texture::Create() {
    return new CXTexture();
}
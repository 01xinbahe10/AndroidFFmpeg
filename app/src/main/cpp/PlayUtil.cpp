//
// Created by xb on 2020/7/23.
//

#include <iostream>
#include "PlayUtil.h"

PlayUtil::PlayUtil() {
    std::cout << "构造函数" << std::endl;
}

PlayUtil::~PlayUtil() {
    std::cout << "析构函数" << std::endl;
}

void PlayUtil::print() {
    std::cout << "我的实例内存地址是:" << this << std::endl;
}
PlayUtil * PlayUtil::get() {
// 局部静态特性的方式实现单实例
    static PlayUtil playUtil;
    return &playUtil;
}

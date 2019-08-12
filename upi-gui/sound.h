#pragma once
#include <dshow.h>

class Sound {
    IGraphBuilder* Sound::graph_builder;
    IMediaControl* media_control; 
    IMediaPosition* media_position;
public:
    Sound();

    // �t�@�C���̓ǂݍ���
    bool load(BSTR file);

    // �Đ�(�񓯊�)
    void play() const;

    // �폜
    void release();
};
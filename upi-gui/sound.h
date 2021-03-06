#pragma once
#include <dshow.h>

class Sound {
    IGraphBuilder* Sound::graph_builder;
    IMediaControl* media_control;
    IMediaPosition* media_position;
public:
    Sound();

    // ファイルの読み込み
    bool load(BSTR file);

    // 再生(非同期)
    void play() const;

    // 削除
    void release();
};
#pragma once
#include <dshow.h>

class Sound {
    IGraphBuilder* Sound::graph_builder;
    IMediaControl* media_control; 
    IMediaPosition* media_position;
public:
    Sound();

    // ƒtƒ@ƒCƒ‹‚Ì“Ç‚İ‚İ
    bool load(BSTR file);

    // Ä¶(”ñ“¯Šú)
    void play() const;

    // íœ
    void release();
};
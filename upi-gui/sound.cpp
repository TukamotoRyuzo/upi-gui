#include "sound.h"

#pragma comment(lib,"strmiids.lib") 

Sound::Sound() : graph_builder(NULL), media_control(NULL), media_position(NULL) {
  
}

// ƒtƒ@ƒCƒ‹‚Ì“Ç‚Ýž‚Ý
bool Sound::load(BSTR file) {
    HRESULT result = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&graph_builder);

    if (FAILED(result)) {
        return false;
    }

    graph_builder->QueryInterface(IID_IMediaControl, (LPVOID*)&media_control);
    graph_builder->QueryInterface(IID_IMediaPosition, (LPVOID*)&media_position);
    result = media_control->RenderFile(file);

    if (FAILED(result)) {
        return false;
    }

    return true;
}

// Ä¶
void Sound::play() const {
    media_position->put_CurrentPosition(0);
    media_control->Run();
}

void Sound::release() {
    if (media_position) {
        media_position->Release();
    }

    if (media_control) {
        media_control->Release();
    }

    if (graph_builder) {
        graph_builder->Release();
    }
}

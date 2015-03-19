#pragma once

namespace CCPlus {
    // OpenGL Context
    void* createGLContext();
    void destroyGLContext(void*);

    // delete file
    void removeFile(const std::string& fileName, bool removeDir=false);

    void copyAssets();
}
#ifdef __IOS__
extern "C" {
    typedef void (*ResultVideoFrameCallback)(void* ctx, const char* bgra, int width, int height, int bytesPerRow, int rotation);
    typedef void (*ResultAudioCallback)(void* ctx, const char* audio, int len, float time);
    typedef void (*RawImageDataCallback)(void* ctx, const uint8_t* data, uint32_t length);

    const char* assetsLibraryExt(const char* url);
    void getAssetsLibraryImage(const char* url, void* ctx, RawImageDataCallback callback);
}

#endif

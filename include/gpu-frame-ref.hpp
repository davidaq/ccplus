#pragma once

class GPUFrameRef {
private:
    uint16_t myGpuContextCounter;
    GPUFrameImpl* ptr = 0;
    int* reference = 0;

    void ref();
    void deref();
public:
    GPUFrameRef();
    GPUFrameRef(GPUFrameImpl*);
    GPUFrameRef(const GPUFrameRef& o);
    ~GPUFrameRef();

    const GPUFrameRef& operator = (const GPUFrameRef& o);

    inline operator GPUFrameImpl* () const {
        return ptr;
    }

    inline GPUFrameImpl* operator -> () const {
        return ptr;
    }

    inline GPUFrameImpl& operator * () const {
        return *ptr;
    }
    inline operator bool() const {
        return ptr && myGpuContextCounter == gpuContextCounter;
    }
};

typedef GPUFrameRef GPUFrame;

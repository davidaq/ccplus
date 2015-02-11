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
    operator GPUFrameImpl* () const ;
    GPUFrameImpl* operator -> () const;
    GPUFrameImpl& operator * () const;
    operator bool() const;
};

typedef GPUFrameRef GPUFrame;

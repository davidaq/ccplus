#include "gpu-frame.hpp"

using namespace CCPlus;

GPUFrameRef::GPUFrameRef() {
}

GPUFrameRef::GPUFrameRef(GPUFrameImpl* ptr_) {
    if(ptr_) {
        myGpuContextCounter = gpuContextCounter;
        ptr = ptr_;
        reference = new int;
        *reference = 0;
        ref();
    }
}

GPUFrameRef::GPUFrameRef(const GPUFrameRef& o) {
    myGpuContextCounter = o.myGpuContextCounter;
    reference = o.reference;
    ptr = o.ptr;
    ref();
}

const GPUFrameRef& GPUFrameRef::operator = (const GPUFrameRef& o) {
    deref();
    myGpuContextCounter = o.myGpuContextCounter;
    reference = o.reference;
    ptr = o.ptr;
    ref();
    return *this;
}

GPUFrameRef::~GPUFrameRef() {
    deref();
}

namespace CCPlus {
    Lock refSyncLock;
};

void GPUFrameRef::ref() {
    if(reference) {
        refSyncLock.lock();
        ++(*reference);
        refSyncLock.unlock();
    }
}

void GPUFrameRef::deref() {
    if(reference) {
        refSyncLock.lock();
        int c = --(*reference);
        refSyncLock.unlock();
        if(c <= 0) {
            if(ptr)
                delete ptr;
            ptr = 0;
            delete reference;
            reference = 0;
        }
    }
}

GPUFrameRef::operator GPUFrameImpl* () const {
    return ptr;
}

GPUFrameImpl* GPUFrameRef::operator -> () const {
    return ptr;
}

GPUFrameImpl& GPUFrameRef::operator * () const {
    return *ptr;
}

GPUFrameRef::operator bool() const {
    if(!ptr)
        return false;
    if(myGpuContextCounter != gpuContextCounter)
        return false;
    return true;
};

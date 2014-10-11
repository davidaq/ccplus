#include "frame.hpp"

using namespace CCPlus;

Frame::Frame() {

}

Frame::~Frame() {

}

Frame Frame::gpu2cpu() {
    if(image.empty()) {

    }
}

Frame Frame::cpu2gpu() {
    if(!textureID) {
        
    }
}

void Frame::write(const std::string& s) {

}

void Frame::read(const std::string& r) {
    gpu2cpu();
}

#pragma once

#include <opencv2/opencv.hpp>

#include <functional>

#include "config.hpp"
#include "object.hpp"

namespace CCPlus {
    class Frame;
}

typedef uint (*BLENDER_CORE)(uint, uint, uint, uint);

class CCPlus::Frame : public CCPlus::Object {
public:
    Frame(const std::string& filepath);
    Frame(const cv::Mat& _image, const cv::Mat& _audio);
    Frame(const cv::Mat&);
    Frame(const std::vector<int16_t>&);
    Frame(int width, int height);
    Frame();
    ~Frame() {
        //image.release();
        //audio.release();
    }

    static Frame emptyFrame(int, int);
    
    void write(const std::string&, int quality=90, bool inMemory=CCPlus::MEMORY_FS);
    
    int getWidth() const;
    int getHeight() const;
    int getImageChannels() const;
    bool empty() const;

    /**
     * Overlay image then merge audio
     */
    void mergeFrame(const Frame& f, int mode = 0); 
    void mergeAudio(const Frame& f);

    /** 
     * Track matte 
     * take @frame's properties as this image's opacity
     */
    void trackMatte(const Frame& frame, int trkMat = 0);

    void setBlackBackground();
    
    cv::Mat& getImage();
    const cv::Mat& getImage() const;
    void setImage(const cv::Mat&);

    cv::Mat& getAudio();
    const cv::Mat& getAudio() const;
    void setAudio(const cv::Mat&);
    void setAudio(const std::vector<int16_t>&);

    void addAlpha(const std::vector<unsigned char>&);

    int getXMin() const;
    int getYMin() const;
    int getXMax() const;
    int getYMax() const;

    void setXMin(int _x) {xMin = _x;};
    void setYMin(int _x) {yMin = _x;};
    void setXMax(int _x) {xMax = _x;};
    void setYMax(int _x) {yMax = _x;};

    int getAnchorAdjustX() const;
    int getAnchorAdjustY() const;
    void setAnchorAdjustX(int);
    void setAnchorAdjustY(int);

private:
    /**
     * Put @input *UNDER* this image
     * REQUIRE: img must be a RGBA image
     */
    //void overlayImage(const cv::Mat& input, BLENDER_CORE blend);
    void overlayImage(const Frame& input, BLENDER_CORE blend);
    void rotateCWRightAngle(int angle);
    void to4Channels();

    /*
     * Effective boundary
     */
    int yMin = -1, yMax = 0x7fffffff, xMin = -1, xMax = 0x7fffffff;

    cv::Mat image;
    cv::Mat audio; 

    int anchorAdjustX = 0, anchorAdjustY = 0;
};

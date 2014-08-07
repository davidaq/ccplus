#pragma once

#include <opencv2/opencv.hpp>

#include "config.hpp"
#include "object.hpp"

namespace CCPlus {
    class Frame;
}

class CCPlus::Frame : public CCPlus::Object {
public:
    Frame(const std::string& filepath);
    Frame(const cv::Mat& _image, const cv::Mat& _audio);
    Frame(const cv::Mat&);
    Frame(const std::vector<int16_t>&);
    Frame(int width, int height);
    Frame();

    static Frame emptyFrame(int, int);
    
    void write(const std::string&, int quality=90, bool inMemory=CCPlus::MEMORY_FS);
    
    int getWidth() const;
    int getHeight() const;
    int getImageChannels() const;

    /**
     * Overlay image then merge audio
     */
    void mergeFrame(const Frame& f); 

    void setWhiteBackground();
    
    cv::Mat& getImage();
    const cv::Mat& getImage() const;
    void setImage(const cv::Mat&);

    cv::Mat& getAudio();
    const cv::Mat& getAudio() const;
    void setAudio(const cv::Mat&);
    void setAudio(const std::vector<int16_t>&);
    
private:
    /**
     * Put @input *UNDER* this image
     * REQUIRE: img must be a RGBA image
     */
    void overlayImage(const cv::Mat& input);
    void rotateCWRightAngle(int angle);
    void to4Channels();

    cv::Mat image;
    cv::Mat audio; 
};

#include "include/cc_ccme_ccplus_VideoInfo.h"
#include "include/jniutil.h"
#include "video-decoder.hpp"

using namespace CCPlus;

namespace JNI_VIDEOINFO {
    VideoDecoder* currentDecoder = 0;
};
using namespace JNI_VIDEOINFO;

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    loadInfo
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_VideoInfo_loadInfo
(JNIEnv *env, jobject, jstring path) {
    currentDecoder = new VideoDecoder(J2STR(path));
}

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getWidth
(JNIEnv *, jobject) {
    if(currentDecoder) {
        return currentDecoder->getVideoInfo().rwidth;
    }
    return 0;
}

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getHeight
(JNIEnv *, jobject) {
    if(currentDecoder) {
        return currentDecoder->getVideoInfo().rheight;
    }
    return 0;
}

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getDuration
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getDuration
(JNIEnv *, jobject) {
    if(currentDecoder) {
        return (int)currentDecoder->getVideoInfo().duration * 1000;
    }
    return 0;
}

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    releaseLoadInfo
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_VideoInfo_releaseLoadInfo
(JNIEnv *, jobject) {
    if(currentDecoder) {
        delete currentDecoder;
        currentDecoder = 0;
    }
}

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    loadThumb
 * Signature: (Ljava/lang/String;II)[I
 */
JNIEXPORT jintArray JNICALL Java_cc_ccme_ccplus_VideoInfo_loadThumb
(JNIEnv *env, jobject, jstring path, jint width, jint height) {
    VideoDecoder decoder(J2STR(path));
    decoder.seekTo(decoder.getVideoInfo().duration * 0.3);
    if(decoder.decodeImage() > 0) {
        cv::Mat img = decoder.getDecodedImage().image;
        if(!img.empty()) {
            int rw = width, rh = height;
            if(rw > img.cols) {
                rh = rh * img.cols / rw;
                rw = img.cols;
            }
            if(rh > img.rows) {
                rw = rw * img.rows / rh;
                rh = img.rows;
            }
            img = img(cv::Rect((img.cols - rw) / 2, (img.rows - rh) / 2, rw, rh));
            cv::resize(img, img, cv::Size(width, height));
            jintArray ret = env->NewIntArray(img.cols * img.rows);
            env->SetIntArrayRegion(ret, 0, img.cols * img.rows, (int*)img.clone().data);
            return ret;
        }
    }
    return NULL;
}

#include <android/log.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include "include/cc_ccme_ccplus_CCPlay.h"
#include "include/jniutil.h"
#include "ccplay.hpp"

using namespace CCPlus;
/*
 * Class:     cc_ccme_ccplus_CCPlay
 * Method:    play
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlay_play
(JNIEnv *env, jclass, jint key, jstring target, jboolean block) {
    void setupPlayer(JNIEnv*);
    setupPlayer(env);
    CCPlay::play(key, J2STR(target), block);
}

/*
 * Class:     cc_ccme_ccplus_CCPlay
 * Method:    stop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlay_stop
(JNIEnv *, jclass) {
    CCPlay::stop();
}

/*
 * Class:     cc_ccme_ccplus_CCPlay
 * Method:    setBufferDuration
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlay_setBufferDuration
(JNIEnv *, jclass, jint bufferDuration) {
    CCPlay::setBufferDuration(bufferDuration);
}

namespace JNI_CCPLAY {
    bool attached = false;
    JavaVM* jvm;
    jclass jCCPlayClass;
    jmethodID jPlay, jProgress;
}
using namespace JNI_CCPLAY;

void playerInterface(
        int key,
        float currentTime,
        const unsigned char* imageData, int imageWidth, int imageHeight, 
        const unsigned char* audioData, int audioLen,
        float bgmVolume
        );
void progressInterface(int key, int precent);

void setupPlayer(JNIEnv* env) {
    if(attached)
        return;
    attached = true;

    env->GetJavaVM(&jvm);
    jCCPlayClass = env->FindClass("cc/ccme/ccplus/CCPlay");
    if(!jCCPlayClass) {
        __android_log_print(ANDROID_LOG_ERROR, "CCPlus", "Can not find interface class cc.ccme.ccplus.CCPlay");
        exit(0);
    }
    jPlay = env->GetStaticMethodID(jCCPlayClass, "playerInterfaceCall", "(IF[III[BF)V");
    jProgress = env->GetStaticMethodID(jCCPlayClass, "progressInterfaceCall", "(II)V");

    CCPlay::attachPlayerInterface(playerInterface);
    CCPlay::attachProgressInterface(progressInterface);
}

static inline void releaseJEnv(JNIEnv* jEnv) {
    jvm->DetachCurrentThread();
}

static inline JNIEnv* initInterface() {
    JNIEnv* jEnv;
    jvm->AttachCurrentThread(&jEnv, 0);
    return jEnv;
}

void playerInterface(
        int key,
        float currentTime,
        const unsigned char* imageData, int imageWidth, int imageHeight, 
        const unsigned char* audioData, int audioLen,
        float bgmVolume
        ) {

    JNIEnv* jEnv = initInterface();
    if(!imageData && !audioData) {
        jEnv->CallStaticVoidMethod(jCCPlayClass, jPlay, key, 0, 0, 0, 0, 0, 0, 0);
    } else {
        int imgLen = imageWidth * imageHeight;
        jintArray jImgData = jEnv->NewIntArray(imgLen);
        jEnv->SetIntArrayRegion(jImgData, 0, imgLen, (int*)imageData);
        jbyteArray jAudData = jEnv->NewByteArray(audioLen);
        jEnv->SetByteArrayRegion(jAudData, 0, audioLen, (jbyte*)audioData);
        jEnv->CallStaticVoidMethod(jCCPlayClass, jPlay, key, currentTime, jImgData, imageWidth, imageHeight, jAudData, bgmVolume);
    }
    releaseJEnv(jEnv);
}

void progressInterface(int key, int percent) {
    JNIEnv* jEnv = initInterface();
    jEnv->CallStaticVoidMethod(jCCPlayClass, jProgress, key, percent);
    releaseJEnv(jEnv);
}

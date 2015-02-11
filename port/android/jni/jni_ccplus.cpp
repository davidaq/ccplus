#include <android/log.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include "include/cc_ccme_ccplus_CCPlus.h"
#include "include/jniutil.h"
#include "ccplus.hpp"

using namespace CCPlus;

namespace JNI_CCPLUS {
    JavaVM* jvm;
    jclass jInterfaceClass;
    jmethodID jCreateGLContext, jDestroyGLContext, jCopyAssets;
}
using namespace JNI_CCPLUS;

void findMethods(JNIEnv* env) {
    if(!jInterfaceClass) {
        env->GetJavaVM(&jvm);
        jInterfaceClass = env->FindClass("cc/ccme/ccplus/CCPlus");
        if(!jInterfaceClass) {
            __android_log_print(ANDROID_LOG_ERROR, "CCPlus", "Can not find interface class cc.ccme.ccplus.CCPlus");
            exit(0);
        }
        jCreateGLContext = env->GetStaticMethodID(jInterfaceClass, "createGLContext", "()V");
        jDestroyGLContext = env->GetStaticMethodID(jInterfaceClass, "destroyGLContext", "()V");
        jCopyAssets = env->GetStaticMethodID(jInterfaceClass, "copyAssets", "()V");
    }
}
/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setAudioSampleRate
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setAudioSampleRate
(JNIEnv *, jclass, jint sampleRate) {
    setAudioSampleRate(sampleRate);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setAssetsPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setAssetsPath
(JNIEnv *env, jclass, jstring path) {
    setAssetsPath(J2STR(path));
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setCollectorThreadsNumber
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setCollectorThreadsNumber
(JNIEnv *, jclass, jint num) {
    setCollectorThreadsNumber(num);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setFrameRate
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setFrameRate
(JNIEnv *, jclass, jint fps) {
    setFrameRate(fps);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    getFrameRate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_getFrameRate
(JNIEnv *, jclass) {
    return getFrameRate();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    go
 * Signature: (IILjava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_go
(JNIEnv *env, jclass, jint index, jint fps, jstring inputPath, jstring outputPath, jint mode) {
    findMethods(env);
    RenderTarget target(index);
    target.fps = fps;
    target.inputPath = J2STR(inputPath);
    target.outputPath = J2STR(outputPath);
    target.mode = mode;
    go(target);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    stop
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_stop
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    target.stop();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    waitFinish
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_waitFinish
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    target.waitFinish();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    getProgress
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_getProgress
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    return target.getProgress();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isActive
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isActive
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    return target.isActive();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isPending
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isPending
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    return target.isPending();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isProcessing
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isProcessing
(JNIEnv *, jclass, jint index) {
    RenderTarget target(index);
    return target.isProcessing();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    onPause
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_onPause
(JNIEnv *, jclass) {
    onPause();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    onResume
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_onResume
(JNIEnv *, jclass) {
    onResume();
}

static inline void releaseJEnv(JNIEnv* jEnv) {
    jvm->DetachCurrentThread();
}

static inline JNIEnv* initInterface() {
    JNIEnv* jEnv;
    jvm->AttachCurrentThread(&jEnv, 0);
    return jEnv;
}

extern "C" {
    void jniCCPlusCreateGLContext();
    void jniCCPlusDestroyGLContext();
    void jniCCPlusCopyAssets();
}

void jniCCPlusCreateGLContext() {
    __android_log_print(ANDROID_LOG_VERBOSE, "CCPlus", "Trying to create gl context");
    JNIEnv* jEnv = initInterface();
    jEnv->CallStaticVoidMethod(jInterfaceClass, jCreateGLContext);
    releaseJEnv(jEnv);
}

void jniCCPlusDestroyGLContext() {
    JNIEnv* jEnv = initInterface();
    __android_log_print(ANDROID_LOG_VERBOSE, "CCPlus", "Trying to destroy gl context");
    jEnv->CallStaticVoidMethod(jInterfaceClass, jDestroyGLContext);
    releaseJEnv(jEnv);
}

void jniCCPlusCopyAssets() {
    JNIEnv* jEnv = initInterface();
    jEnv->CallStaticVoidMethod(jInterfaceClass, jCopyAssets);
    releaseJEnv(jEnv);
}

#include <android/log.h>
#include <string>
#include <stdlib.h>
#include <cstring>
#include "ccplus-jni.h"
#include "ccplus.hpp"
#include "jniutil.hpp"
#include "parallel-executor.hpp"

using namespace CCPlus;

JavaVM* jvm;
jclass jInterfaceClass;
jmethodID jCreateGLContext, jDestroyGLContext, jReadAsset;

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
        jReadAsset = env->GetStaticMethodID(jInterfaceClass, "readAssets", "(Ljava/lang/String;)[B");
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
 * Method:    setOutputPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setOutputPath
(JNIEnv *env, jclass, jstring path) {
    setOutputPath(J2STR(path));
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setRenderMode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setRenderMode
(JNIEnv *, jclass, jint mode) {
    setRenderMode(mode);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    initContext
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_initContext
(JNIEnv *env, jclass, jstring tml) {
    findMethods(env);
    initContext(J2STR(tml));
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    releaseContext
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_releaseContext
(JNIEnv *, jclass, jboolean forceReleaseCache) {
    releaseContext(forceReleaseCache);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    render
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_render
(JNIEnv *, jclass) {
    render();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    numberOfFrames
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_numberOfFrames
(JNIEnv *, jclass) {
    return numberOfFrames();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    getRenderProgress
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_getRenderProgress
(JNIEnv *, jclass) {
    return getRenderProgress();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    waitRender
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_waitRender
(JNIEnv *, jclass) {
    waitRender();
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    generateTML
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_generateTML
(JNIEnv *env, jclass, jstring configFile, jboolean halfSize) {
    findMethods(env);
    std::string cfgFile = J2STR(configFile);
    pthread_join(ParallelExecutor::runInNewThread([cfgFile, halfSize] {
        generateTML(cfgFile, halfSize);
    }), 0);
}

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    testShaders
 * Signature: ()V
 */
extern "C" {
    void jniCCPlusCreateGLContext();
    void jniCCPlusDestroyGLContext();
}
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_testShaders
(JNIEnv *env, jclass, jstring path) {
    __android_log_print(ANDROID_LOG_VERBOSE, "CCPlus", "Test Shaders");
    findMethods(env);
    CCPLUS_TEST(J2STR(path));
}





void releaseJEnv(JNIEnv* jEnv) {
    jvm->DetachCurrentThread();
}

JNIEnv* initInterface() {
    JNIEnv* jEnv;
    jvm->AttachCurrentThread(&jEnv, 0);
    return jEnv;
}

void jniCCPlusCreateGLContext() {
    JNIEnv* jEnv = initInterface();
    __android_log_print(ANDROID_LOG_VERBOSE, "CCPlus", "Trying to create gl context");
    jEnv->CallStaticVoidMethod(jInterfaceClass, jCreateGLContext);
    releaseJEnv(jEnv);
}

void jniCCPlusDestroyGLContext() {
    JNIEnv* jEnv = initInterface();
    __android_log_print(ANDROID_LOG_VERBOSE, "CCPlus", "Trying to destroy gl context");
    jEnv->CallStaticVoidMethod(jInterfaceClass, jDestroyGLContext);
    releaseJEnv(jEnv);
}

void jniCCPlusReadAsset(const char* name, void** dest, int* sz) {
    JNIEnv* jEnv = initInterface();
    jbyteArray jResult = (jbyteArray)jEnv->CallStaticObjectMethod(jInterfaceClass, jReadAsset, str2jstring(jEnv, name));
    if(jResult) {
        *sz = jEnv->GetArrayLength(jResult);
        if(*sz > 0) {
            jbyte* jPtr = jEnv->GetByteArrayElements(jResult, 0);
            *dest = malloc(*sz);
            memcpy(*dest, jPtr, *sz);
        }
    }
    releaseJEnv(jEnv);
}


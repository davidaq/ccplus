/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class cc_ccme_ccplus_CCPlus */

#ifndef _Included_cc_ccme_ccplus_CCPlus
#define _Included_cc_ccme_ccplus_CCPlus
#ifdef __cplusplus
extern "C" {
#endif
#undef cc_ccme_ccplus_CCPlus_PREVIEW_MODE
#define cc_ccme_ccplus_CCPlus_PREVIEW_MODE 0L
#undef cc_ccme_ccplus_CCPlus_FINAL_MODE
#define cc_ccme_ccplus_CCPlus_FINAL_MODE 1L
/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setAudioSampleRate
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setAudioSampleRate
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setAssetsPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setAssetsPath
  (JNIEnv *, jclass, jstring);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setCollectorThreadsNumber
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setCollectorThreadsNumber
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setFrameRate
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setFrameRate
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    getFrameRate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_getFrameRate
  (JNIEnv *, jclass);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setOutputPath
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setOutputPath
  (JNIEnv *, jclass, jstring);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    setRenderMode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_setRenderMode
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    go
 * Signature: (IILjava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_go
  (JNIEnv *, jclass, jint, jint, jstring, jstring, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    stop
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_stop
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    waitFinish
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_waitFinish
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    getProgress
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_CCPlus_getProgress
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isActive
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isActive
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isPending
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isPending
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    isProcessing
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cc_ccme_ccplus_CCPlus_isProcessing
  (JNIEnv *, jclass, jint);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    onPause
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_onPause
  (JNIEnv *, jclass);

/*
 * Class:     cc_ccme_ccplus_CCPlus
 * Method:    onResume
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_CCPlus_onResume
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
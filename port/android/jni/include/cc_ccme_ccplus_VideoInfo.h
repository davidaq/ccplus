/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class cc_ccme_ccplus_VideoInfo */

#ifndef _Included_cc_ccme_ccplus_VideoInfo
#define _Included_cc_ccme_ccplus_VideoInfo
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    loadInfo
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_VideoInfo_loadInfo
  (JNIEnv *, jobject, jstring);

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getWidth
  (JNIEnv *, jobject);

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getHeight
  (JNIEnv *, jobject);

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    getDuration
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_cc_ccme_ccplus_VideoInfo_getDuration
  (JNIEnv *, jobject);

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    releaseLoadInfo
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cc_ccme_ccplus_VideoInfo_releaseLoadInfo
  (JNIEnv *, jobject);

/*
 * Class:     cc_ccme_ccplus_VideoInfo
 * Method:    loadThumb
 * Signature: (Ljava/lang/String;II)[I
 */
JNIEXPORT jintArray JNICALL Java_cc_ccme_ccplus_VideoInfo_loadThumb
  (JNIEnv *, jobject, jstring, jint, jint);

#ifdef __cplusplus
}
#endif
#endif

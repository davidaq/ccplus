#pragma once
#include <string>

#define J2STR(X) jstring2str(env, X).c_str()
#define STR2J(X) str2jstring(env, X.c_str())

static inline jstring str2jstring(JNIEnv* env,const char* str) {  
    return env->NewStringUTF(str);
} 

static inline std::string jstring2str(JNIEnv* env, jstring jstr) {     
    const char* nstr = env->GetStringUTFChars(jstr, 0);
    std::string ret(nstr);
    env->ReleaseStringUTFChars(jstr, nstr);
    return ret;
} 

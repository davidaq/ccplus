#ifndef __CCPLUS__H__
#define __CCPLUS__H__

// CCPlus interface for C

#ifdef __cplusplus
extern "C" {
#endif 

void* CCPlusInitContext(const char* tmlPath, const char* storagePath, int fps);
void CCPlusReleaseContext(void* ctxHandle);
void CCPlusRenderPart(void* ctxHandle, float start, float length);
void CCPlusEncodeVideo(void* ctxHandle, float start, float length);

#ifdef __cplusplus
}
#endif

#endif

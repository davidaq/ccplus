#ifdef VERSION_100

#ifdef IS_VERTEX_SHADER
    precision mediump float;
    precision mediump int;
#endif

#ifdef IS_FRAGMENT_SHADER
    precision lowp float;
    precision mediump int;
#endif

#endif


#ifdef VERSION_120

#define precision 
#define highp
#define mediump
#define lowp

#endif

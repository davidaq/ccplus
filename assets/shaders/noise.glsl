float snoise2D(vec2 i){
    mediump vec2 co = i;
    mediump float ret = fract(dot(co.xy, vec2(1.0, 1.0))) + 1.0;
    ret = fract(ret * ret) + 1.0;
    ret = fract(ret * ret) + 1.2;
    ret = fract(ret * ret) + 1.4;
    ret = fract(ret * ret) + 1.6;
    return fract(fract(ret * 153.019) * 7.47);
}

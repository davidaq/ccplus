float snoise2D(vec2 i){
    mediump vec2 co = i;
    mediump float ret = dot(co.xy, vec2(12.9898,78.233));
    ret = sin(ret);
    return fract(fract(ret * 153.019) * 7.47);
}

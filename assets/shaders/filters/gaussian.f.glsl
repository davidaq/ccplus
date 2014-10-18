varying vec2 xy;
varying vec2 nbs[12];

uniform int ksize;
uniform sampler2D tex;
uniform float gWeights[12];

void main() {
    //for (int i = 0; i < ksize; i++) {
    //    vec4 col = texture2D(tex, xy + nbs[i]) + texture2D(tex, xy - nbs[i]);
    //    gl_FragColor += gWeights[i] * col;
    //}
    gl_FragColor += gWeights[0] * (texture2D(tex, xy + nbs[0]) + texture2D(tex, xy - nbs[0]));
    gl_FragColor += gWeights[1] * (texture2D(tex, xy + nbs[1]) + texture2D(tex, xy - nbs[1]));
    gl_FragColor += gWeights[2] * (texture2D(tex, xy + nbs[2]) + texture2D(tex, xy - nbs[2]));
    gl_FragColor += gWeights[3] * (texture2D(tex, xy + nbs[3]) + texture2D(tex, xy - nbs[3]));
    gl_FragColor += gWeights[4] * (texture2D(tex, xy + nbs[4]) + texture2D(tex, xy - nbs[4]));
    gl_FragColor += gWeights[5] * (texture2D(tex, xy + nbs[5]) + texture2D(tex, xy - nbs[5]));
    gl_FragColor += gWeights[6] * (texture2D(tex, xy + nbs[6]) + texture2D(tex, xy - nbs[6]));
    gl_FragColor += gWeights[7] * (texture2D(tex, xy + nbs[7]) + texture2D(tex, xy - nbs[7]));
    gl_FragColor += gWeights[8] * (texture2D(tex, xy + nbs[8]) + texture2D(tex, xy - nbs[8]));
    gl_FragColor += gWeights[9] * (texture2D(tex, xy + nbs[9]) + texture2D(tex, xy - nbs[9]));
    gl_FragColor += gWeights[10] * (texture2D(tex, xy + nbs[10]) + texture2D(tex, xy - nbs[10]));
    gl_FragColor += gWeights[11] * (texture2D(tex, xy + nbs[11]) + texture2D(tex, xy - nbs[11]));
    gl_FragColor += gWeights[11] * (texture2D(tex, xy + nbs[11]) + texture2D(tex, xy - nbs[11]));
}

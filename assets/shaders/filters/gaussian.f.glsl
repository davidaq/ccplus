varying vec2 nbs[16];

uniform int ksize;
uniform sampler2D tex;
uniform float gWeights[8];

void main() {
    gl_FragColor  = gWeights[0] * (texture2D(tex, nbs[0]) + texture2D(tex, nbs[15]));
    gl_FragColor += gWeights[1] * (texture2D(tex, nbs[1]) + texture2D(tex, nbs[14]));
    gl_FragColor += gWeights[2] * (texture2D(tex, nbs[2]) + texture2D(tex, nbs[13]));
    gl_FragColor += gWeights[3] * (texture2D(tex, nbs[3]) + texture2D(tex, nbs[12]));
    gl_FragColor += gWeights[4] * (texture2D(tex, nbs[4]) + texture2D(tex, nbs[11]));
    gl_FragColor += gWeights[5] * (texture2D(tex, nbs[5]) + texture2D(tex, nbs[10]));
    gl_FragColor += gWeights[6] * (texture2D(tex, nbs[6]) + texture2D(tex, nbs[ 9]));
    gl_FragColor += gWeights[7] * (texture2D(tex, nbs[7]) + texture2D(tex, nbs[ 8]));
}

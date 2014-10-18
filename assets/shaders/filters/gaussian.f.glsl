varying vec2 xy;
varying vec2 nbs[12];

uniform int ksize;
uniform sampler2D tex;
uniform float gWeights[12];

void main() {
    for (int i = 0; i < ksize; i++) {
        vec4 col = texture2D(tex, xy + nbs[i]) + texture2D(tex, xy - nbs[i]);
        gl_FragColor += gWeights[i] * col;
    }
}

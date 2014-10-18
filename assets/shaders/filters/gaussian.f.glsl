varying vec2 xy;
varying vec2 nbs[12];

uniform int ksize;
uniform sampler2D tex;
uniform float gWeights[100];

void main() {
    //const float gWeights[9] = float[9](
    //    0.10855,
    //    0.13135,
    //    0.10406,
    //    0.07216,
    //    0.04380,
    //    0.02328,
    //    0.01083,
    //    0.00441,
    //    0.00157
    //);
    for (int i = 0; i < ksize; i++) {
        vec4 col = texture2D(tex, xy + nbs[i]).rgba + texture2D(tex, xy - nbs[i]).rgba;
        //vec3 col = texture2D(tex, nbs[i]).rgb + texture2D(tex, nbs[i + ksize]).rgb;
        gl_FragColor.rgba += gWeights[i] * col;
    }
}

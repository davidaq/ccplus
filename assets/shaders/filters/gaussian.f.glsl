#version 120

varying vec2 xy;
varying vec2 nbs[25];
varying float cnt;

uniform sampler2D tex;
uniform float gWeights[100];

void main() {
    gl_FragColor = vec4(0, 0, 0, 1);
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
    //int ksize = int(cnt);
    for (int i = 0; i < cnt; i++) {
        vec3 col = texture2D(tex, xy + nbs[i]).rgb + texture2D(tex, xy - nbs[i]).rgb;
        //vec3 col = texture2D(tex, nbs[i]).rgb + texture2D(tex, nbs[i + ksize]).rgb;
        gl_FragColor.rgb += gWeights[i] * col;
    }
}

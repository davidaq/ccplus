varying vec4 nbs[12];

uniform int ksize;
uniform sampler2D tex;
uniform float gWeights[12];

void main() {
    gl_FragColor  = gWeights [0] * (texture2D(tex, nbs [0].xy) + texture2D(tex, nbs [0].zw));
    gl_FragColor += gWeights [1] * (texture2D(tex, nbs [1].xy) + texture2D(tex, nbs [1].zw));
    gl_FragColor += gWeights [2] * (texture2D(tex, nbs [2].xy) + texture2D(tex, nbs [2].zw));
    gl_FragColor += gWeights [3] * (texture2D(tex, nbs [3].xy) + texture2D(tex, nbs [3].zw));
    gl_FragColor += gWeights [4] * (texture2D(tex, nbs [4].xy) + texture2D(tex, nbs [4].zw));
    gl_FragColor += gWeights [5] * (texture2D(tex, nbs [5].xy) + texture2D(tex, nbs [5].zw));
    gl_FragColor += gWeights [6] * (texture2D(tex, nbs [6].xy) + texture2D(tex, nbs [6].zw));
    gl_FragColor += gWeights [7] * (texture2D(tex, nbs [7].xy) + texture2D(tex, nbs [7].zw));
    gl_FragColor += gWeights [8] * (texture2D(tex, nbs [8].xy) + texture2D(tex, nbs [8].zw));
    gl_FragColor += gWeights [9] * (texture2D(tex, nbs [9].xy) + texture2D(tex, nbs [9].zw));
    gl_FragColor += gWeights[10] * (texture2D(tex, nbs[10].xy) + texture2D(tex, nbs[10].zw));
    gl_FragColor += gWeights[11] * (texture2D(tex, nbs[11].xy) + texture2D(tex, nbs[11].zw));
}

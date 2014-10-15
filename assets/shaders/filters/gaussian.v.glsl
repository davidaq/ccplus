#version 120

attribute vec4 vertex_position;

varying vec2 xy;
varying vec2 nbs[9];

uniform vec2 pixelOffset;
uniform int ksize;

void main() {
    const float gOffsets[9] = float[9](
        0.66293,
        2.47904,
        4.46232,
        6.44568,
        8.42917,
        10.41281,
        12.39664,
        14.38070,
        16.36501
    );
    for(int i = 0; i < 9; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        nbs[i] = texCoordOffset;
    }; 

    gl_Position = vertex_position;
    xy = gl_Position.xy * 0.5 + 0.5;

}

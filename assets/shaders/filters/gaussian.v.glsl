attribute vec4 vertex_position;

varying vec2 xy;
varying vec2 nbs[12];

uniform vec2 pixelOffset;
uniform int ksize;
uniform float gOffsets[12];

void main() {
    gl_Position = vertex_position;
    xy = gl_Position.xy * 0.5 + 0.5;
    for(int i = 0; i < 12; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        nbs[i] = texCoordOffset;
    }; 
}

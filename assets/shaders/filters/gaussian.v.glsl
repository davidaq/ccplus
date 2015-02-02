attribute vec4 vertex_position;

varying vec2 nbs[16];

uniform vec2 pixelOffset;
uniform int ksize;
uniform float gOffsets[8];

void main() {
    gl_Position = vertex_position;
    vec2 xy = gl_Position.xy * 0.5 + 0.5;
    for(int i = 0; i < 8; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        nbs[i] = xy + texCoordOffset;
        nbs[15 - i] = xy - texCoordOffset;
    }; 
}

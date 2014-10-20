attribute vec4 vertex_position;

varying vec4 nbs[8];

uniform vec2 pixelOffset;
uniform int ksize;
uniform float gOffsets[8];

void main() {
    gl_Position = vertex_position;
    vec2 xy = gl_Position.xy * 0.5 + 0.5;
    for(int i = 0; i < 8; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        nbs[i].xy = xy + texCoordOffset;
        nbs[i].zw = xy - texCoordOffset;
    }; 
}

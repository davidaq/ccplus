#version 120

attribute vec4 vertex_position;

varying vec2 xy;

uniform mat3 T;

void main() {
    gl_Position = vertex_position;

    vec3 tmp = gl_Position.xyz;
    tmp = (tmp + 1.0) / 2.0;
    tmp = T * tmp;
    xy = tmp.xy;
}

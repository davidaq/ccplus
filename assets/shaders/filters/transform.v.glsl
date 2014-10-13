#version 120

attribute vec4 vertex_position;

varying vec2 xy;

uniform mat3 T;

void main() {
    xy = (vertex_position.xy + 1.0) / 2.0;
    vec3 tmp = vec3(vertex_position.xy, 1);
    tmp = T * tmp;
    tmp.xy = tmp.xy / tmp.z;
    gl_Position.xy = tmp.xy;

}

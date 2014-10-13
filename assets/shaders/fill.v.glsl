#version 120

attribute vec4 vertex_position;

varying vec2 xy;

void main() {
    gl_Position = vertex_position;

    xy = (gl_Position.xy + 1.0) / 2.0;
}

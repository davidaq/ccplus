#version 120

attribute vec4 vertex_position;

varying vec2 px;

void main() {
    gl_Position = vertex_position;
    px = vertex_position.xy;
}

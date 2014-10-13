#version 120

varying vec2 xy;

uniform sampler2D tex_up;

void main() {
    gl_FragColor = texture2D(tex_up, xy);
}

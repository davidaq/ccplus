#version 120

varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    gl_FragColor = texture2D(tex_up, xy).a * texture2D(tex_down, xy);
}

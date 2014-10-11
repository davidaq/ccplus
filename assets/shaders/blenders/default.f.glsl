#version 120

uniform sampler2D tex;

varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 c_up = texture2D(tex1, xy);
    vec4 c_down = texture2D(tex2, xy);
    gl_FragColor = c_up + c_down;
}
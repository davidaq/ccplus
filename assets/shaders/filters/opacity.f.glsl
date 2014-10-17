#version 100
precision lowp float;

varying vec2 xy;

uniform sampler2D tex;
uniform float opa;

void main() {
    gl_FragColor = texture2D(tex, xy);
    gl_FragColor.a = gl_FragColor.a * opa;
}

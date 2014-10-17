#version 100
precision lowp float;

varying vec2 xy;

uniform sampler2D tex;

void main() {
    gl_FragColor = texture2D(tex, xy);
}

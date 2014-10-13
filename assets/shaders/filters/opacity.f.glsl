#version 120

varying vec2 xy;

uniform sampler2D tex;
uniform float opa;

void main() {
    gl_FragColor = texture2D(tex, xy);
    gl_FragColor.a = gl_FragColor.a * opa;
    gl_FragColor = vec4(1, 0, 0, 0.5);
}

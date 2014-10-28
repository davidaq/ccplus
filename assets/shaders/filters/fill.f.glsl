varying vec2 xy;

uniform vec3 col;

void main() {
    gl_FragColor.a = 1.0;
    gl_FragColor.rgb = col;
}

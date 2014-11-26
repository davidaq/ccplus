varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    gl_FragColor = texture2D(tex_up, xy) + texture2D(tex_down, xy);
}

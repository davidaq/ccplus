varying vec2 xy;

uniform sampler2D tex;
uniform float alpha;

void main() {
    gl_FragColor = texture2D(tex, xy) * alpha;
}


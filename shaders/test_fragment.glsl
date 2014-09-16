#version 120

uniform sampler2D tex;

varying vec2 px;

uniform mat2 T;

void main() {
    gl_FragColor = texture2D(tex, 
            T * (px + vec2(1.0, 1.0)) / 2.0);
}

#version 120

varying vec2 px;

attribute vec4 vertex_position;
uniform mat3 M; // This is transformation 
uniform mat3 Ortho; // Set by GPU_worker

void main() {
    vec3 v = vec3(vertex_position.xy, 1.0);
    v = M * v;
    v = v / v.z;
    gl_Position.xyz = Ortho * v;
    px = gl_Position.xy;
}

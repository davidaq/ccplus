attribute vec4 vertex_position;

varying vec2 xy;

uniform vec2 transition;
uniform mat3 trans;
uniform vec4 src_dst_size;

void main() {
    xy = vertex_position.xy * 0.5 + 0.5;

    vec3 tmp = vec3(xy * src_dst_size.xy, 1.0);
    tmp = trans * tmp;
    tmp /= tmp.z;
    tmp.xy += transition;
    
    tmp.xy /= src_dst_size.zw;
    gl_Position.xy = tmp.xy * 2.0 - 1.0;
    gl_Position.zw = vec2(0.0, 1.0);
}

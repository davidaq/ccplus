attribute vec4 vertex_position;

varying vec2 xy;

uniform float zoom;
uniform mat4 trans;
uniform vec4 src_dst_size; //src.width, src.height, dst.width, dst.height

void main() {
    xy = vertex_position.xy * 0.5 + 0.5;
    
    vec4 tmp = vec4(xy, 0.0, 1.0);
    tmp.xy *= src_dst_size.xy;
    tmp = trans * tmp;
    tmp /= tmp.w;

    // Simple camera view
    float ratio = (tmp.z + zoom) / zoom;

    tmp.xy /= src_dst_size.zw;
    gl_Position.xy = tmp.xy * 2.0 - 1.0;
    gl_Position.zw = vec2(0.0, ratio);
}

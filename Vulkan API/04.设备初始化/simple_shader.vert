#version 450

vec2 positions[3] = vec2[] ( // 三角形的顶点位置
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); // 顶点位置转换为齐次坐标
}
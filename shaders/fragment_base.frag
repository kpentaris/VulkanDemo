#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor; // input from vertex shader
layout(location = 0) out vec4 outColor; // location specifies the index of the framebuffer

void main() {
    outColor = vec4(fragColor, 1.0);
}

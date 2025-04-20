#version 460 core

out vec4 color;
uniform float timeValue;
uniform vec3 lightColor;

void main() {
    color = vec4(0.5 + 0.5 * sin(timeValue), 1.0, 1.0, 0.7);
}

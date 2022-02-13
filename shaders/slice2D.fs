#version 450

in vec3 fragTexCoord;

uniform sampler3D volumeData;
uniform vec4 vertexColor;

out vec4 color;

void main() {
    color = vertexColor + texture(volumeData, fragTexCoord);
}
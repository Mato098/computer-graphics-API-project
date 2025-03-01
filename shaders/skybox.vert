#version 450

layout(binding = 0, std140) uniform Camera {
	mat4 projection;
	mat4 view;
	vec3 position;
} camera;

layout (location = 0) in vec3 position;

out vec3 TexCoords;

void main()
{
    TexCoords = position;
	/*
	mat4 no_translation_view = camera.view; // another option as opposed to scaling
	no_translation_view[3][0] = 0.0;
	no_translation_view[3][1] = 0.0;
	no_translation_view[3][2] = 0.0;
	no_translation_view[3][3] = 1.0;
	*/
    gl_Position = camera.projection * camera.view * vec4(position * 500, 1.0);
}  
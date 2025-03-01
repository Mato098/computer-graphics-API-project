#version 450
out vec4 final_color;

in vec3 TexCoords;

uniform samplerCube skybox;

layout(location = 5) uniform float game_end = 0.0f;

void main()
{    
    final_color = texture(skybox, TexCoords);
    
    //toon shading
    float shades = 12.0;
    final_color.r = floor(final_color.r * shades) / shades;
    final_color.g = floor(final_color.g * shades) / shades;
    final_color.b = floor(final_color.b * shades) / shades;

    if (game_end < 0.0f) {
		final_color += vec4(0.2f, 1.0f, 0.2f, 0.0f) * game_end * 0.7;
	}
	else if (game_end > 0.0f) {
		final_color += vec4(1.0f, 1.0f, 1.0f, 0.0f) * game_end * 0.7;
	}
}
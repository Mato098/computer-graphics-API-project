#version 450

//----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// There are no custom input variables (except the in-build ones).

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------

layout(binding = 8) uniform sampler2D health_tex;
// The final output color.
layout(location = 0) out vec4 final_color;
layout(location = 2) in vec2 fs_texture_coordinate;
layout(location = 6) in float player_hp;
layout(location = 8) uniform float game_time;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float x = gl_FragCoord.x / 720 * 3 - game_time / 90000 ;
	float y = (gl_FragCoord.y) / 480 * 3 - game_time / 40000 ;
    final_color = texture(health_tex, vec2(x, y), 0);
}

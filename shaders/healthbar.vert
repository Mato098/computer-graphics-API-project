#version 450

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// There are no custom input variables (except the in-build ones).

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// There are no custom output variables (except the in-build ones).

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------

const vec2 triangle[3] = {
	vec2(-0.5, -0.5),
	vec2(0.5, -0.5),
	vec2(0.0, 0.5)
};

layout(location = 6) uniform float player_hp;
layout(location = 7) uniform float enemy_hp;
layout(location = 2) in vec2 texture_coordinate;
layout(location = 2) out vec2 fs_texture_coordinate;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{	
	vec2 enemy_triangle[6] = {
		vec2(-0.9 + 1.8 * enemy_hp, 0.9),
		vec2(-0.9, 0.9),
		vec2(-0.9, 0.8),
		vec2(-0.9 + 1.8 * enemy_hp, 0.9),
		vec2(-0.9, 0.8),
		vec2(-0.9 + 1.8 * enemy_hp, 0.8)
};
vec2 player_triangle[6] = {
		vec2(-0.5 + 1.0 * player_hp, -0.9),
		vec2(-0.5, -0.9),
		vec2(-0.5, -0.8),
		vec2(-0.5 + 1.0 * player_hp, -0.9),
		vec2(-0.5, -0.8),
		vec2(-0.5 + 1.0 * player_hp, -0.8)
};
	fs_texture_coordinate = texture_coordinate;
	
    gl_Position = vec4(gl_VertexID < 6 ? enemy_triangle[gl_VertexID] : player_triangle[gl_VertexID % 6], 0.0, 1.0);
}
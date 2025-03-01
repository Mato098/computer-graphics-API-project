#version 450

layout(binding = 0, std140) uniform Camera {
    mat4 projection;
    mat4 view;
    vec3 position;
}
camera;

struct Light {
	vec4 position;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
};

layout(binding = 1, std430) buffer Lights {
	Light lights[];
};

layout(binding = 2, std140) uniform Object {
    mat4 model_matrix;
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
}
object;

layout(binding = 3, std140) uniform DirLight {
    vec4 direction;//discard w value of all members
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
}dir_light;

layout(binding = 4) uniform sampler2D albedo_texture;
layout(binding = 5) uniform sampler2D normal_texture;
layout(binding = 6) uniform sampler2D shadow_texture;
layout(binding = 7) uniform samplerCube skybox;

layout(location = 3) uniform bool has_texture = false;
layout(location = 4) uniform float uv_scale = 1.0f;
layout(location = 5) uniform float game_end = 0.0f;
layout(location = 7) uniform bool use_environmental_mapping = false;

layout(location = 0) in vec3 fs_position;
layout(location = 1) in vec3 fs_normal;
layout(location = 2) in vec2 fs_texture_coordinate;

layout(location = 0) out vec4 final_color;

vec4 point_light();
vec4 calculate_sun(vec3 normal, vec3 viewDir);

void main() {
	final_color = vec4(0.0, 0.0, 0.0, 1.0); // for transparent glass effect set w to 0.0 (or lower than 1)
    final_color += point_light();
    final_color += calculate_sun(fs_normal, normalize(camera.position - fs_position));

    if (use_environmental_mapping) {
        vec3 I = normalize(fs_position - camera.position);
        vec3 R = reflect(I, normalize(fs_normal));
        final_color = final_color * 0.6 + vec4(texture(skybox, R).rgb, 1.0) * 0.4;
	}

    //toon shading
    float shades = 8.0;
    final_color.r = floor(final_color.r * shades) / shades;
    final_color.g = floor(final_color.g * shades) / shades;
    final_color.b = floor(final_color.b * shades) / shades;
    if (game_end < 0.0f) {
		final_color += vec4(0.2f, 1.0f, 0.2f, 0.0f) * game_end;
	}
	else if (game_end > 0.0f) {
		final_color += vec4(1.0f, 1.0f, 1.0f, 0.0f) * game_end;
	}

}

vec4 calculate_sun(vec3 normal, vec3 viewDir)
{
    float sun_power = 1.5;

	vec2 flipped_uv = vec2(fs_texture_coordinate.x, (1.0 - fs_texture_coordinate.y)) * uv_scale;
	vec3 lightDir = normalize(-vec3(dir_light.direction));
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.1);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	// combine results
	vec3 ambient = vec3(dir_light.ambient_color) * vec3(texture(albedo_texture, flipped_uv));

	vec3 diffuse = vec3(dir_light.diffuse_color) * diff * vec3(texture(albedo_texture, flipped_uv)) * sun_power;

    float shadow_ratio = max(-dot(normal, lightDir), 0.08);

    diffuse -= min(shadow_ratio * vec3(texture(shadow_texture, flipped_uv)), 0.3);

	vec3 specular = vec3(dir_light.specular_color) * spec * vec3(texture(albedo_texture, flipped_uv));
	return vec4(ambient + diffuse + specular, 0.0);
}

vec4 point_light(){
	vec2 flipped_uv = vec2(fs_texture_coordinate.x, 1.0 - fs_texture_coordinate.y) * uv_scale;

    vec3 final_color = vec3(0.0f, 0.0f, 0.0f);

    Light curr_light;
    for (int i = 0; i < lights.length(); i++) {
		if (lights[i].position.w == 0.0){
			break;
		}
		curr_light = lights[i];

        vec3 light_vector = curr_light.position.xyz - fs_position * curr_light.position.w;
        vec3 L = normalize(light_vector);
        vec3 N = normalize(fs_normal);

        vec3 normal = texture(normal_texture, flipped_uv).xyz * 2.0 - 1.0;
        vec3 T = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
        vec3 B = normalize(cross(normal, T));
        N = normalize(T * normal.x + B * normal.y + N * normal.z);


        vec3 E = normalize(camera.position - fs_position);
        vec3 H = normalize(L + E);

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0001);

        vec3 ambient = object.ambient_color.rgb * curr_light.ambient_color.rgb;
        vec3 diffuse = object.diffuse_color.rgb * (has_texture ? texture(albedo_texture, flipped_uv).rgb : vec3(1.0)) * curr_light.diffuse_color.rgb;
        vec3 specular = object.specular_color.rgb * curr_light.specular_color.rgb;

        
        float shadow_ratio = min(-dot(normal, L), 0.01);

        diffuse -= min(shadow_ratio * vec3(texture(shadow_texture, flipped_uv)), 0.8);

        vec3 color = ambient.rgb + NdotL * diffuse.rgb + pow(NdotH, object.specular_color.w) * specular;

        if (curr_light.position.w == 1.0) {
            color /= (dot(light_vector, light_vector));
         }
        final_color += color;
    }

    final_color = final_color / (final_color + 1.0);       // tone mapping
    final_color = pow(final_color, vec3(1.0 / 2.2)); // gamma correction

    return vec4(final_color, 0.0);
}
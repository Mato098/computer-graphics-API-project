#pragma once

#include "application.hpp"
#include "data.hpp"
#include "collisions.h"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::make_shared;

Application::ShotLine::ShotLine(glm::vec3 pos1, glm::vec3 pos2, float aliveTime, glm::vec3 color, float thickness = 1.0f) {
	this->pos1 = pos1;
	this->pos2 = pos2;
	spawn_time = global_time;
	this->color = color;
	max_time_alive = aliveTime;

	vertices = {
		pos1, pos2, pos2 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness,
		pos2, pos2 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness, pos1 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness,
		pos1 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness, pos1, pos2,
		pos2 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness, pos1 + 0.01f * glm::vec3(0.0f, 1.0f, 0.0f) * thickness, pos2
	};
	GLuint vtx_vbo;
	glCreateBuffers(1, &vtx_vbo);
	glNamedBufferStorage(vtx_vbo, sizeof(float) * 3 * vertices.size(), vertices.data(), 0);
	glCreateVertexArrays(1, &vao);
	glVertexArrayVertexBuffer(vao, 0, vtx_vbo, 0, sizeof(float) * 3);
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
}
void Application::ShotLine::Draw() {
	if (spawn_time + max_time_alive < global_time) {
		return;
	}
	glm::vec3 fin_color = color;// *(1.0f - (global_time - spawn_time / max_time_alive) / 10000.0f);

	glBindVertexArray(vao);
	glUniform3f(5, fin_color.r, fin_color.g, fin_color.b);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

}

Application::Object::Object(const std::filesystem::path col, const std::filesystem::path normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries,
	ObjectUBO* ubo = NULL, std::string namee = "no name") {
	name = namee;
	color_texture = load_texture_2d(col);
	normal_texture = load_texture_2d(normal);
	geometries->push_back(make_shared<Geometry>(Geometry::from_file(_geometry, false)));
	geometry_ptr = geometries->at(geometries->size() - 1);

	if (ubo != NULL) {
		obj_ubo = *ubo;
	}
	glCreateBuffers(1, &obj_ubo_id);
	glNamedBufferStorage(obj_ubo_id, sizeof(ObjectUBO), &obj_ubo, GL_DYNAMIC_STORAGE_BIT);
}
Application::Object::Object(GLuint col, GLuint normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries, ObjectUBO* ubo = NULL, std::string namee = "no name") {
	name = namee;
	color_texture = col;
	normal_texture = normal;
	geometries->push_back(make_shared<Geometry>(Geometry::from_file(_geometry, false)));
	geometry_ptr = geometries->at(geometries->size() - 1);

	if (ubo != NULL) {
		obj_ubo = *ubo;
	}
	glCreateBuffers(1, &obj_ubo_id);
	glNamedBufferStorage(obj_ubo_id, sizeof(ObjectUBO), &obj_ubo, GL_DYNAMIC_STORAGE_BIT);
}
Application::Object::Object(GLuint col, GLuint normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries, std::string namee, glm::vec3 bbox_min, glm::vec3 bbox_max, int type) {
	name = namee;
	color_texture = col;
	normal_texture = normal;
	geometries->push_back(make_shared<Geometry>(Geometry::from_file(_geometry, false)));
	geometry_ptr = geometries->at(geometries->size() - 1);
	this->BBOX_max = bbox_max;
	this->BBOX_min = bbox_min;
	this->type = type;

	glCreateBuffers(1, &obj_ubo_id);
	glNamedBufferStorage(obj_ubo_id, sizeof(ObjectUBO), &obj_ubo, GL_DYNAMIC_STORAGE_BIT);
}
void Application::Object::Draw(glm::vec3 lookDirection = glm::vec3(0.0f, 0.0f, 0.0f)) {
	if (type == PLAYER || type == INVISIBLE_ENEMY || type == INVISIBLE) return;
	if (fireCycle_pos > 0.0f) {
		Cycle_fire();
		fireCycle_pos = (application->last_fire_time + 200.f - global_time) / 90.0f;
	}
	glBindTextureUnit(4, color_texture);
	glBindTextureUnit(5, normal_texture);
	glBindTextureUnit(6, application->shadow_texture);

	if (use_environmental_mapping) {
		glBindTextureUnit(7, application->skybox);
		glUniform1i(glGetUniformLocation(application->main_program, "use_environmental_mapping"), true);
	}
	glUniform1f(glGetUniformLocation(application->main_program, "uv_scale"), uv_scale);

	if (parent != NULL) {
		obj_ubo.model_matrix = parent->obj_ubo.model_matrix;
		obj_ubo.model_matrix = glm::translate(obj_ubo.model_matrix, position);
		obj_ubo.model_matrix = glm::translate(obj_ubo.model_matrix, fire_offset);
		if (type != PISTOL) {
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		}
	}
	else {
		obj_ubo.model_matrix = glm::mat4(1.0f);

		obj_ubo.model_matrix = glm::translate(obj_ubo.model_matrix, position);
		if (type != PISTOL) {
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		obj_ubo.model_matrix = glm::translate(obj_ubo.model_matrix, fire_offset);
		if (lookDirection != glm::vec3(0.0f, 0.0f, 0.0f)) {
			rotate_to_look(lookDirection);
		}
	}
	glNamedBufferSubData(obj_ubo_id, 0, sizeof(ObjectUBO), &obj_ubo);
	application->bindBuffers(obj_ubo_id, false, false, true);

	geometry_ptr->draw();
	for (auto child : children) {
		child->Draw();
	}
	if (use_environmental_mapping) {
		glUniform1i(glGetUniformLocation(application->main_program, "use_environmental_mapping"), false);
	}
}

void Application::Object::Fire() {//cycle the position
	fireCycle_pos = 1;
}
void Application::Object::Cycle_fire() {
	fire_offset = glm::vec3(0.0f, 0.0f, 0.03f) * -(pow(fireCycle_pos, 2) / (pow(fireCycle_pos, 2) + pow((1 - fireCycle_pos), 2)));
}

void Application::Object::rotate_to_look(glm::vec3 lookDirection) {
	glm::vec3 y_rot_axis = glm::vec3(0.0f, 1.0f, 0.0f);
	float y_rot_amount = glm::orientedAngle(glm::normalize(glm::vec2(lookDirection.x, lookDirection.z)), glm::vec2(0.0f, 1.0f));
	obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, y_rot_amount, y_rot_axis);
	//flip the x rotation axes
	glm::vec3 x_rot_axis = glm::normalize(glm::cross(lookDirection, y_rot_axis));
	float x_rot_amount = glm::orientedAngle(lookDirection, glm::vec3(0.0f, 1.0f, 0.0f), x_rot_axis);
	//obj_ubo.model_matrix = glm::rotate(obj_ubo.model_matrix, x_rot_amount, x_rot_axis); //TODO: fix this
}
Application::Object::~Object() {
	geometry_ptr.reset();
}
void Application::Object::CalculateBBOX(std::filesystem::path pth) {//cannot get to the vertices :(
	std::ifstream file(pth);
	std::string line;
	float max_x = -10000.f;
	float max_y = -10000.f;
	float max_z = -10000.f;
	float min_x = 10000.f;
	float min_y = 10000.f;
	float min_z = 10000.f;

	glm::vec3 vertice;
	while (std::getline(file, line)) {
		if (line.substr(0, 2) == "v ") {
			std::istringstream s(line.substr(2));
			glm::vec3 v;
			s >> v.x;
			s >> v.y;
			s >> v.z;
			if (v.x > max_x) max_x = v.x;
			if (v.y > max_y) max_y = v.y;
			if (v.z > max_z) max_z = v.z;
			if (v.x < min_x) min_x = v.x;
			if (v.y < min_y) min_y = v.y;
			if (v.z < min_z) min_z = v.z;
		}
	}
	BBOX_max = glm::vec3(max_x, max_y, max_z);
	BBOX_min = glm::vec3(min_x, min_y, min_z);
}


GLuint Application::Object::load_texture_2d(const std::filesystem::path filename) {
	int width, height, channels;
	unsigned char* data = stbi_load(filename.generic_string().data(), &width, &height, &channels, 4);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);

	glTextureStorage2D(texture, std::log2(width), GL_RGBA8, width, height);

	glTextureSubImage2D(texture,
		0,                         //
		0, 0,                      //
		width, height,             //
		GL_RGBA, GL_UNSIGNED_BYTE, //
		data);

	stbi_image_free(data);

	glGenerateTextureMipmap(texture);

	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

GLuint load_texture_2d(const std::filesystem::path filename) {
	int width, height, channels;
	unsigned char* data = stbi_load(filename.generic_string().data(), &width, &height, &channels, 4);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);

	glTextureStorage2D(texture, std::log2(width), GL_RGBA8, width, height);

	glTextureSubImage2D(texture,
		0,                         //
		0, 0,                      //
		width, height,             //
		GL_RGBA, GL_UNSIGNED_BYTE, //
		data);

	stbi_image_free(data);

	glGenerateTextureMipmap(texture);

	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
	: PV112Application(initial_width, initial_height, arguments) {
	this->width = initial_width;
	this->height = initial_height;
	cam_width = initial_width;
	cam_height = initial_height;
	MyCamera camera = MyCamera();
	application = this;


	images_path = lecture_folder_path / "images";
	objects_path = lecture_folder_path / "objects";

	// --------------------------------------------------------------------------
	//  Load/Create Objects
	// --------------------------------------------------------------------------
	geometries.push_back(make_shared<Sphere>());
	// You can use from_file function to load a Geometry from .obj file
	geometries.push_back(make_shared<Geometry>(Geometry::from_file(objects_path / "bunny.obj")));

	cube_bbox_test_obj = Object(images_path / "floor" / "aerial_grass_rock_diff_2k.jpg",
		images_path / "floor" / "aerial_grass_rock_nor_gl_2k.jpg", objects_path / "cube.obj", &geometries);
	cube_bbox_test_obj.BBOX_min = glm::vec3(-1.f, -1.f, -1.f);
	cube_bbox_test_obj.BBOX_max = glm::vec3(1.f, 1.f, 1.f);

	importLevel();

	std::cout << "Press   F   to enter fly mode" << std::endl;
	std::cout << "Press  ESC  to exit the game" << std::endl;

	pistol = Object(images_path / "g18_basecolor_1k.png", images_path / "g18_opengl_n_1k.png", objects_path / "g18_base.obj", &geometries);
	pistol.type = PISTOL;
	pistol.children.push_back(make_shared<Object>(images_path / "g18_basecolor_1k.png", images_path / "g18_opengl_n_1k.png", objects_path / "g18_slide.obj", &geometries));
	pistol.children.at(0).get()->parent = &pistol;
	pistol.children.at(0).get()->type = PISTOL;

	Object playerHitbox = Object(0, 0, objects_path / "cube.obj", &geometries);

	std::vector<std::filesystem::path> faces = {
			images_path / "px.png",
			images_path / "nx.png",
			images_path / "py.png",
			images_path / "ny.png",
			images_path / "pz.png",
			images_path / "nz.png"
	};
	skybox = loadCubemap(faces);
	GLuint vtx_vbo;
	glCreateBuffers(1, &vtx_vbo);
	glNamedBufferStorage(vtx_vbo, sizeof(float) * 3 * 6 * 6, &skyboxVertices, 0);
	glCreateVertexArrays(1, &skybox_vao);
	glVertexArrayVertexBuffer(skybox_vao, 0, vtx_vbo, 0, sizeof(float) * 3);
	glEnableVertexArrayAttrib(skybox_vao, 0);
	glVertexArrayAttribFormat(skybox_vao, 0, 3, GL_FLOAT, false, 0);
	glVertexArrayAttribBinding(skybox_vao, 0, 0);

	sphere = geometries[0];
	bunny = geometries[1];

	// --------------------------------------------------------------------------
	// Initialize UBO Data
	// --------------------------------------------------------------------------
	// Camera
	// 
	camera_ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f, 1000.0f);
	camera_ubo.view = glm::lookAt(glm::vec3(camera.getEyePosition()), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	light_ubo.position = glm::vec4(0.0f, 5.0f, -4.0f, 1.0f);
	light_ubo.ambient_color = glm::vec4(2.5f, 0.0f, 0.2f, 1.0f);
	light_ubo.diffuse_color = glm::vec4(2.0f, 2.0f, 1.0f, 1.0f);
	light_ubo.specular_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	objects_ubos.push_back({ .model_matrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)), glm::vec3(light_ubo.position)),
							.ambient_color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
							.diffuse_color = glm::vec4(0.0),
							.specular_color = glm::vec4(0.0f) });
	objects_ubos.push_back({ .model_matrix = glm::mat4(1.0f),
							.ambient_color = glm::vec4(0.0f),
							.diffuse_color = glm::vec4(1.0f),
							.specular_color = glm::vec4(0.0f) });

	LightUBO dir_light_ubo = {
		.position = glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f),
		.ambient_color = glm::vec4(0.01f, 0.01f, 0.01f, 0.0f),
		.diffuse_color = glm::vec4(0.96f, 0.91f, 0.73f, 0.0f),
		.specular_color = glm::vec4(0.98f - 0.5f, 0.98f - 0.5f, 0.96f - 0.5f, 0.0f)
	};

	// --------------------------------------------------------------------------
	// Create Buffers
	// --------------------------------------------------------------------------
	glCreateBuffers(1, &camera_buffer);
	glNamedBufferStorage(camera_buffer, sizeof(CameraUBO), &camera_ubo, GL_DYNAMIC_STORAGE_BIT);

	//create dummy light array
	for (int i = 0; i < 100; i++) {
		lights.push_back({ .position = glm::vec4(0.0f),
										.ambient_color = glm::vec4(0.0f),
										.diffuse_color = glm::vec4(0.0f),
										.specular_color = glm::vec4(0.0f) });
	}
	glCreateBuffers(1, &lights_buffer);
	glNamedBufferStorage(lights_buffer, sizeof(LightUBO) * 100, lights.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateBuffers(1, &objects_buffer);
	glNamedBufferStorage(objects_buffer, sizeof(ObjectUBO) * objects_ubos.size(), objects_ubos.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateBuffers(1, &dir_light_buffer);
	glNamedBufferStorage(dir_light_buffer, sizeof(LightUBO), &dir_light_ubo, GL_DYNAMIC_STORAGE_BIT);


	glCreateFramebuffers(1, &framebuffer_2);
	glCreateTextures(GL_TEXTURE_2D, 1, &framebuffer_2_color);
	glCreateTextures(GL_TEXTURE_2D, 1, &framebuffer_2_depth);

	glNamedFramebufferTexture(framebuffer_2, GL_COLOR_ATTACHMENT0, framebuffer_2_color, 0);
	glNamedFramebufferTexture(framebuffer_2, GL_DEPTH_ATTACHMENT, framebuffer_2_depth, 0);

	glTextureStorage2D(framebuffer_2_depth, 1, GL_DEPTH_COMPONENT32F, width, height);
	glTextureStorage2D(framebuffer_2_color, 1, GL_RGBA32F, width, height);

	const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	// ...
	glNamedFramebufferDrawBuffers(framebuffer_2, 1, draw_buffers);

	glNamedFramebufferTexture(framebuffer_2, GL_COLOR_ATTACHMENT0, framebuffer_2_color, 0);
	glNamedFramebufferTexture(framebuffer_2, GL_DEPTH_ATTACHMENT, framebuffer_2_depth, 0);


	glCreateVertexArrays(1, &healthbar_vao);

	compile_shaders();
}

Application::~Application() {
	delete_shaders();

	glDeleteBuffers(1, &camera_buffer);
	glDeleteBuffers(1, &lights_buffer);
	glDeleteBuffers(1, &objects_buffer);
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

void Application::delete_shaders() {}

void Application::compile_shaders() {
	delete_shaders();
	main_program = create_program(lecture_shaders_path / "main.vert", lecture_shaders_path / "main.frag");
	skybox_program = create_program(lecture_shaders_path / "skybox.vert", lecture_shaders_path / "skybox.frag");
	shot_program = create_program(lecture_shaders_path / "main.vert", lecture_shaders_path / "shot.frag");
	postprocess_program = create_program(lecture_shaders_path / "postprocess.vert", lecture_shaders_path / "postprocess.frag");
	healthbar_program = create_program(lecture_shaders_path / "healthbar.vert", lecture_shaders_path / "healthbar.frag");
}

void Application::update(float delta) {

	global_time += delta;

	if (gamestate == -1 || gamestate == 1) {
		if (global_time - gamestate_change_time > 2000 && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
		return;
	}
	if (robot_hp <= 0) {
		gamestate = 1;
		gamestate_change_time = global_time;
		return;
	}
	if (player_hp <= 0) {
		gamestate = -1;
		gamestate_change_time = global_time;
		return;
	}

	last_ads_time += delta;

	parse_input(delta);

	if (!flyMode) { //set player height
		if (camera.getEyePosition().y < 2.0f) camera.setEyePosition(glm::vec4(camera.getEyePosition().x, 2.0f, camera.getEyePosition().z, 1.0f));
		if (camera.getEyePosition().y > 2.0f) camera.setEyePosition(glm::vec4(camera.getEyePosition().x, 2.0f, camera.getEyePosition().z, 1.0f));
	}

	camera_ubo.position = camera.getEyePosition();
	pistol.position = isADS ? glm::vec3(camera.getEyePosition()) + camera.getLookDirection() * 0.2f +
		glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f) - glm::vec3(0.0f, 0.1f, 0.0f) :
		glm::vec3(camera.getEyePosition()) + camera.getLookDirection() * 0.2f - glm::vec3(0.0f, 0.1f, 0.0f);

	pistol.rotation = camera.getLookDirection();
	move_vector = glm::vec2(0, 0);

	//rotate robot to face player
	float y_rot_amount = glm::orientedAngle(glm::normalize(glm::vec2(camera.getEyePosition().x, camera.getEyePosition().z)), glm::vec2(0.0f, 1.0f));
	robot_top.rotation = glm::vec3(0.0f, y_rot_amount, 0.0f);

	if (robot_hp < 50) {
		robot_min_time_between_bursts = 1.0f;
		robot_max_time_between_bursts = 5.0f;
	}
	else if (robot_hp < 30) {
		robot_min_time_between_bursts = 0.5f;
		robot_max_time_between_bursts = 3.0f;
	}
	else if (robot_hp < 10) {
		robot_min_time_between_bursts = 0.1f;
	}

	//manage robot shooting
	if (robot_hp < 30.0f) {
		robot_accuracy = 1.5f;
	}
	if (!robot_in_burst) {
		robot_time_till_next_burst -= delta / 1000.0f;

		if (robot_time_till_next_burst < 0.0f) {
			robot_in_burst = true;
			robot_time_till_next_burst = glm::linearRand(robot_min_time_between_bursts, robot_max_time_between_bursts);
			robot_shots_until_end_of_burst = robot_burst_size;
			if (rand() % 10 < 3.0f + (100.0f - robot_hp) / 15.0f) {
				robot_shots_until_end_of_burst = 10;
			}
		}
	}

	if (robot_in_burst) {
		if (robot_time_till_next_shot_in_burst < 0.0f) {
			robot_time_till_next_shot_in_burst = robot_time_between_shots_in_burst;
			robot_shots_until_end_of_burst--;
			if (robot_shots_until_end_of_burst == 0) {
				robot_in_burst = false;
			}
			else {
				glm::vec3 rotated_pos = glm::vec3(0.0f, 6.0f, 0.0f) + glm::cross(glm::normalize(glm::vec3(camera.getEyePosition())), glm::vec3(0.0f, 1.0f, 0.0f)) * 3.0f;
				glm::vec3 eye_pos = glm::vec3(camera.getEyePosition().x / camera.getEyePosition().w, camera.getEyePosition().y / camera.getEyePosition().w, camera.getEyePosition().z / camera.getEyePosition().w);

				Shoot(rotated_pos, eye_pos - rotated_pos, vibrantColors.at(rand() % vibrantColors.size()), ENEMY, robot_accuracy, 10.0f, 100.0f);
			}
		}
		else {
			robot_time_till_next_shot_in_burst -= delta / 1000.0f;
		}
	}
}

void Application::parse_input(float delta) {
	float move_speed = 0.01f * delta;
	float collision_bump_speed = move_speed * 0;
	float collision_check_distance = 0.4f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if (!checkCollisions(glm::vec3(camera.getEyePosition()) + camera.getLookDirection() * collision_check_distance,
			player_bbox1, player_bbox2)) {
			move_vector.x = 1;
			camera.setEyePosition(camera.getEyePosition() + glm::vec4(camera.getLookDirection(), 0.0f) * move_speed);
		}
		else {
			camera.setEyePosition(camera.getEyePosition() - glm::vec4(camera.getLookDirection(), 0.0f) * collision_bump_speed);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		if (!checkCollisions(glm::vec3(camera.getEyePosition()) - camera.getLookDirection() * collision_check_distance,
			player_bbox1, player_bbox2)) {
			move_vector.x = -1;
			camera.setEyePosition(camera.getEyePosition() - glm::vec4(camera.getLookDirection(), 0.0f) * move_speed);
		}
		else {
			camera.setEyePosition(camera.getEyePosition() + glm::vec4(camera.getLookDirection(), 0.0f) * collision_bump_speed);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		if (!checkCollisions(glm::vec3(camera.getEyePosition()) -
			glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)) * collision_check_distance, player_bbox1, player_bbox2)) {
			move_vector.y = -1;
			camera.setEyePosition(camera.getEyePosition() -
				glm::vec4(glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)), 0.0f) * move_speed);
		}
		else {
			camera.setEyePosition(camera.getEyePosition() +
				glm::vec4(glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)), 0.0f) * collision_bump_speed);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (!checkCollisions(glm::vec3(camera.getEyePosition()) +
			glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)) * collision_check_distance, player_bbox1, player_bbox2)) {
			move_vector.y = 1;
			camera.setEyePosition(camera.getEyePosition() +
				glm::vec4(glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)), 0.0f) * move_speed);
		}
		else {
			camera.setEyePosition(camera.getEyePosition() -
				glm::vec4(glm::cross(camera.getLookDirection(), glm::vec3(0.0f, 1.0f, 0.0f)), 0.0f) * collision_bump_speed);
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (global_time - last_fire_time - delta > 400.0f) {
			pistol.children.at(0).get()->Fire();
			last_fire_time = global_time;
			Shoot(glm::vec3(camera.getEyePosition()), camera.getLookDirection(), glm::vec3(1.0, 0.64, 0.0), PLAYER, 1.0f, 1.0f, 90.0f);
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (last_ads_time + delta > 200.0f) {
			last_ads_time = 0.0f;
			isADS = !isADS;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		compile_shaders();
	}
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		game_speed += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		if (game_speed > 0.1f) game_speed -= 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (last_ads_time + delta > 200.0f) {
			last_ads_time = 0.0f;
			flyMode = !flyMode;
		}
	}
}

void Application::render() {
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Configure fixed function pipeline
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_2);
	// Clear attachments
	glClearNamedFramebufferfv(framebuffer_2, GL_COLOR, 0, clear_color);
	glClearNamedFramebufferfv(framebuffer_2, GL_DEPTH, 0, clear_depth);
	// --------------------------------------------------------------------------
	// Update UBOs
	// --------------------------------------------------------------------------
	// Camera
	//camera_ubo.position = glm::vec4(camera.get_eye_position(), 1.0f);

	camera_ubo.projection = glm::perspective(glm::radians(60.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f, 1000.0f);
	camera_ubo.view = glm::lookAt(glm::vec3(camera.getEyePosition()), glm::vec3(camera.getEyePosition()) + glm::normalize(camera.getLookDirection()), glm::vec3(0.0f, 1.0f, 0.0f));
	glNamedBufferSubData(camera_buffer, 0, sizeof(CameraUBO), &camera_ubo);

	// --------------------------------------------------------------------------
	// Draw scene
	// --------------------------------------------------------------------------

	// ------ Skybox ------

	glUseProgram(skybox_program);

	if (gamestate == 1) {
		std::cout << "YOU WON!     press ESC to exit the game" << std::endl;
		glUniform1f(glGetUniformLocation(skybox_program, "game_end"), (global_time - gamestate_change_time) / 3000.0f);
	}
	else if (gamestate == -1) {
		std::cout << "GAME OVER    press ESC to exit the game" << std::endl;
		glUniform1f(glGetUniformLocation(skybox_program, "game_end"), -(global_time - gamestate_change_time) / 3000.0f);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);

	glDepthMask(GL_FALSE);
	glBindVertexArray(skybox_vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Draw objects
	glUseProgram(main_program);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, 0 * 256, sizeof(ObjectUBO));

	glUniform1i(glGetUniformLocation(main_program, "has_texture"), false);
	//sphere->draw();

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, 1 * 256, sizeof(ObjectUBO));

	glUniform1i(glGetUniformLocation(main_program, "has_texture"), true);
	//bunny->draw();


	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, 1 * 256, sizeof(ObjectUBO));


	glUseProgram(shot_program);

	glDisable(GL_CULL_FACE);
	for (auto shot : shotLines) {
		shot.get()->Draw();
	}
	glEnable(GL_CULL_FACE);

	glUseProgram(main_program);

	buildLightsBuffer();
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);

	glUniform1i(glGetUniformLocation(main_program, "has_texture"), true);
	//set directional light parameters
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, dir_light_buffer);
	if (gamestate == 1) {
		glUniform1f(glGetUniformLocation(main_program, "game_end"), (global_time - gamestate_change_time) / 3000);
	}
	else if (gamestate == -1) {
		glUniform1f(glGetUniformLocation(main_program, "game_end"), -(global_time - gamestate_change_time) / 3000);
	}

	ground_obj.Draw();
	for (auto obj : objects) {
		obj.Draw();
	}
	pistol.Draw(camera.getLookDirection());
	glUniform1i(glGetUniformLocation(main_program, "use_environmental_mapping"), true);
	glBindTextureUnit(7, skybox);
	robot_top.Draw();
	glUniform1i(glGetUniformLocation(main_program, "use_environmental_mapping"), false);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);

	glUseProgram(postprocess_program);

	glBindTextureUnit(0, framebuffer_2_color);
	glBindTextureUnit(1, framebuffer_2_depth);

	// Draw the full-screen triangle.
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//render ui using healthbar program (just two squares, one for player and one for enemy hp)
	glUseProgram(healthbar_program);
	glBindVertexArray(healthbar_vao);
	glBindTextureUnit(8, health_texture);

	glUniform1f(6, player_hp / 100.0f);
	glUniform1f(7, robot_hp / 100.0f);
	glUniform1f(8, global_time);

	glDisable(GL_CULL_FACE);

	glDrawArrays(GL_TRIANGLES, 0, 12);
}

void Application::buildLightsBuffer() {
	lights.clear();
	lights.push_back(light_ubo);
	int curr_light_amount = 1;
	for (auto shot : shotLines) {
		if (curr_light_amount > 100) break;
		if (shot.get()->max_time_alive + shot.get()->spawn_time < global_time) continue;
		curr_light_amount++;

		//sample lights along shot path
		for (int i = 0; i < lights_per_shot; i++) {
			LightUBO light = {
				.position = glm::vec4(glm::vec3(
					std::lerp(shot.get()->pos1.x, shot.get()->pos2.x, (float)i / lights_per_shot),
					std::lerp(shot.get()->pos1.y, shot.get()->pos2.y, (float)i / lights_per_shot),
					std::lerp(shot.get()->pos1.z, shot.get()->pos2.z, (float)i / lights_per_shot)), 1.0f),
				.ambient_color = glm::vec4(0.0f),
				.diffuse_color = glm::vec4(shot.get()->color, 0.0f),
				.specular_color = glm::vec4(1.0f)
			};
			lights.push_back(light);
		}
	}
	//std::cout << "lights: " << lights.size() << std::endl;
	//fill the rest to 100
	for (int i = curr_light_amount; i < 100; i++) {
		lights.push_back({ .position = glm::vec4(0.0f),
													.ambient_color = glm::vec4(0.0f),
													.diffuse_color = glm::vec4(0.0f),
													.specular_color = glm::vec4(0.0f) });
	}
	glNamedBufferSubData(lights_buffer, 0, sizeof(LightUBO) * 100, lights.data());
}

void Application::bindBuffers(GLuint obj_ubo, bool camera, bool light, bool texture) {
	if (camera) glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
	if (light) glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, obj_ubo);
	//glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, 1 * 256, sizeof(ObjectUBO));
	glUniform1i(glGetUniformLocation(main_program, "has_texture"), texture);
}

void Application::render_ui() { const float unit = ImGui::GetFontSize(); }

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------

void Application::on_resize(int width, int height) {
	// Calls the default implementation to set the class variables.
	PV112Application::on_resize(width, height);
}

void Application::on_mouse_move(double x, double y) {
	if (x < 0 || x > cam_width || y < 0 || y > cam_height) return;
	camera.onMouseMove(x - cam_width / 2, y - cam_height / 2);
	//reset mouse to center of window
	glfwSetCursorPos(window, cam_width / 2, cam_height / 2);
}
void Application::on_mouse_button(int button, int action, int mods) { /*camera.on_mouse_button(button, action, mods);*/ }
void Application::on_key_pressed(int key, int scancode, int action, int mods) {
	// Calls default implementation that invokes compile_shaders when 'R key is hit.
	//PV112Application::on_key_pressed(key, scancode, action, mods);
}

GLuint Application::loadCubemap(std::vector<std::filesystem::path> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].generic_string().data(), &width, &height, &nrChannels, 4);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA8, width, height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void Application::Shoot(glm::vec3 shoot_origin, glm::vec3 shoot_direction, glm::vec3 color, int exclude_tag, float accuracy, float thickness = 1.0f, float ttl = 0.5f) {
	shoot_direction = glm::normalize(shoot_direction);
	shoot_origin += (!isADS ? glm::vec3(0.01f, -0.04f, 0.01f) : glm::cross(shoot_direction, glm::vec3(0.0f, 1.0f, 0.0f) * 0.1f) + glm::vec3(0.0f, -0.05f, 0.0f)) + shoot_direction * 0.1f; //pistol offset compensation
	glm::vec3 destination = shoot_origin + shoot_direction * 50.0f;

	//add randomness
	destination += glm::vec3((rand() % 100) / 80.0f * (rand() % 2 == 0 ? 1 : -1), (rand() % 100) / 80.0f * (rand() % 2 == 0 ? 1 : -1), (rand() % 100) / 80.0f * (rand() % 2 == 0 ? 1 : -1)) * accuracy;
	Object nearest_obj;
	glm::vec3 nearest_point = glm::vec3(10000.0f, 10000.0f, 10000.0f);

	bool canHitPlayer = true;
	if (exclude_tag == PLAYER) canHitPlayer = false;
	for (auto obj : objects) {
		if (obj.type == exclude_tag || (exclude_tag == ENEMY && obj.type == INVISIBLE_ENEMY)) continue;
		if (obj.BBOX_min == glm::vec3(0.0f) && obj.BBOX_max == glm::vec3(0.0f)) continue;
		glm::vec3 point = isCollidingLineBox(shoot_origin, destination, obj.BBOX_min, obj.BBOX_max);

		if (point != glm::vec3(0.0f, 0.0f, 0.0f)) {
			if (glm::distance(shoot_origin, point) < glm::distance(shoot_origin, nearest_point)) {
				nearest_point = point;
				nearest_obj = obj;
			}
		}
	}
	if (canHitPlayer) { //because player is not in objects(they are immutable as far as i know)
		glm::vec3 point = isCollidingLineBox(shoot_origin, destination, player_bbox1 * 1.5f + glm::vec3(camera.getEyePosition()), player_bbox2 * 1.5f + glm::vec3(camera.getEyePosition()));
		if (point != glm::vec3(0.0f, 0.0f, 0.0f)) {
			if (glm::distance(shoot_origin, point) < glm::distance(shoot_origin, nearest_point)) {
				nearest_point = point;
				nearest_obj = Object();
				nearest_obj.type = PLAYER;
			}
		}
	}
	if (nearest_point != glm::vec3(10000.0f, 10000.0f, 10000.0f)) {
		//std::cout << "hit " << nearest_obj.name << std::endl;
		SpawnShotLine(shoot_origin, nearest_point, color, thickness, ttl);
		if (nearest_obj.type == ENEMY || nearest_obj.type == INVISIBLE_ENEMY) {
			if (nearest_obj.name == "robot_bottom") {
				robot_hp -= 0.5f;
			}
			else if (nearest_obj.name == "robot_collider") {
				robot_hp -= 1.6f;
			}
			else if (nearest_obj.name == "robot_cabin") {
				robot_hp -= 2.0f;
			}
			//std::cout << "robot hp .. " << robot_hp << std::endl;
		}
		else if (nearest_obj.type == PLAYER) {
			if (!flyMode) player_hp -= 3.0f;
			//std::cout << "player hp .. " << player_hp << std::endl;
		}
		return;
	}
	SpawnShotLine(shoot_origin, destination, color, thickness, ttl);
}

void Application::SpawnShotLine(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, float thickness = 1.0f, float ttl = 0.5f) {
	ShotLine shot = ShotLine(pos1, pos2, ttl, color, thickness);
	shotLines.push_back(make_shared<ShotLine>(shot));
}

bool Application::checkCollisions(glm::vec3 target_pos, glm::vec3 BBOX_min, glm::vec3 BBOX_max) {
	for (auto obj : objects) {
		if (obj.BBOX_min == glm::vec3(0.0f) && obj.BBOX_max == glm::vec3(0.0f)) continue;
		if (isCollidingOnMove(target_pos, obj.position, BBOX_min, BBOX_max, obj.BBOX_min, obj.BBOX_max)) {
			//std::cout << "COLLISION " << obj.name << std::endl;
			return true;
		}
	}
	return false;
}
void Application::importLevel() {
	std::cout << "importing level" << std::endl;
	std::cout << "--loading textures" << std::endl;
	GLuint wall_col = load_texture_2d(images_path / "stone_wall_02_diff_2k.jpg");
	GLuint wall_normal = load_texture_2d(images_path / "stone_wall_02_nor_dx_2k.jpg");
	GLuint crate_col = load_texture_2d(images_path / "medieval_wood_diff_1k.jpg");
	GLuint crate_normal = load_texture_2d(images_path / "medieval_wood_nor_dx_1k.jpg");
	GLuint other_col = load_texture_2d(images_path / "rock_08_diff_1k.jpg");
	GLuint other_normal = load_texture_2d(images_path / "rock_08_nor_dx_1k.jpg");
	GLuint steel_col = load_texture_2d(images_path / "SteelBattered_1K_albedo.png");
	GLuint steel_normal = load_texture_2d(images_path / "SteelBattered_1K_normal.png");
	health_texture = load_texture_2d(images_path / "health.png");
	shadow_texture = load_texture_2d(images_path / "shadow.png");
	std::cout << "--textures loaded" << std::endl;

	std::filesystem::path level_path = objects_path / "level";
	int obj_count = 0;
	for (const auto& entry : std::filesystem::directory_iterator(level_path)) {
		if (entry.path().extension() == ".obj") {
			Object object;
			std::string n = entry.path().filename().generic_string();

			if (entry.path().filename().generic_string().substr(0, 4) == "wall") {
				object = Object(wall_col, wall_normal, entry.path(), &geometries, NULL, n);
				object.type = LEVEL;
				object.CalculateBBOX(entry.path());
				object.uv_scale = 5.0f;
				objects.push_back(object);
			}
			else if (entry.path().filename().generic_string().substr(0, 5) == "crate") {
				object = Object(crate_col, crate_normal, entry.path(), &geometries, NULL, n);
				object.type = LEVEL;
				object.CalculateBBOX(entry.path());
				object.uv_scale = 5.0f;
				objects.push_back(object);
			}
			else if (entry.path().filename().generic_string().substr(0, 6) == "chains") {
				object = Object(steel_col, steel_normal, entry.path(), &geometries, NULL, n);
				object.type = LEVEL;
				objects.push_back(object);
			}
			else if (entry.path().filename().generic_string().substr(0, 6) == "ground") {
				ground_obj = Object(images_path / "floor" / "aerial_grass_rock_diff_2k.jpg",
					images_path / "floor" / "aerial_grass_rock_nor_gl_2k.jpg", entry.path(), &geometries, NULL, n);
			}
			else {
				object = Object(other_col, other_normal, entry.path(), &geometries, NULL, n);
				object.type = LEVEL;
				objects.push_back(object);
			}
			obj_count++;
			std::cout << "obj no." << obj_count << " " << object.name << std::endl;
		}
	}
	Object robot_bottom = Object(steel_col, steel_normal, objects_path / "robot_bottom.obj", &geometries, NULL, "robot_bottom");
	robot_bottom.BBOX_min = glm::vec3(-2.f, -2.f, -2.f);
	robot_bottom.BBOX_max = glm::vec3(2.f, 6.f, 2.f);
	robot_bottom.type = ENEMY;
	robot_bottom.use_environmental_mapping = true;
	//robot_bottom.name = "robot_bottom";
	objects.push_back(robot_bottom);

	robot_top = Object(steel_col, steel_normal, objects_path / "robot_top.obj", &geometries, NULL, "robot_top");
	robot_top.BBOX_min = glm::vec3(-2.f, 6.f, -2.f);
	robot_top.BBOX_max = glm::vec3(2.f, 11.5f, 2.f);
	robot_top.type = ENEMY;
	//robot_top.name = "robot_top";

	//make fake object for collisions
	objects.push_back(Object(steel_col, steel_normal, objects_path / "cube.obj", &geometries, "robot_collider", glm::vec3(-2.f, 6.f, -2.f),
		glm::vec3(2.f, 11.5f, 2.f), INVISIBLE_ENEMY));

	Object front_cover = Object(steel_col, steel_normal, objects_path / "robot_top_cabin.obj", &geometries, NULL, "robot_cabin");
	robot_top.children.push_back(make_shared<Object>(front_cover));
	robot_top.children.at(0).get()->parent = &robot_top;
	robot_top.children.at(0).get()->type = ENEMY;
	//objects.push_back(robot_cabin);
	std::cout << "--level imported" << std::endl;
}

// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) 2021-2022 Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################

#pragma once

#include "cube.hpp"
#include "pv112_application.hpp"
#include "sphere.hpp"
#include "teapot.hpp"
#include "MyCamera.hpp"
#include "collisions.h"
//#include "YoinkedCamera.h"



// ----------------------------------------------------------------------------
// UNIFORM STRUCTS
// ----------------------------------------------------------------------------
struct CameraUBO {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec4 position;
};

struct LightUBO {
	glm::vec4 position;
	glm::vec4 ambient_color;
	glm::vec4 diffuse_color;
	glm::vec4 specular_color;
};

struct alignas(256) ObjectUBO {
	glm::mat4 model_matrix;  // [  0 -  64) bytes
	glm::vec4 ambient_color; // [ 64 -  80) bytes
	glm::vec4 diffuse_color; // [ 80 -  96) bytes

	// Contains shininess in .w element
	glm::vec4 specular_color; // [ 96 - 112) bytes
};

// Constants
const float clear_color[4] = { 0.0, 0.0, 0.0, 1.0 };
const float clear_depth[1] = { 1.0 };

static float game_speed = 0.17f;
static bool flyMode = false;
static int lights_per_shot = 6;
static float global_time = 0.0f;
static int gamestate = 0;
static float gamestate_change_time = 0.0f;

class Application : public PV112Application {

	// ----------------------------------------------------------------------------
	// Variables
	// ----------------------------------------------------------------------------

	float last_update_time = 0.0f;
	MyCamera camera;
	float last_ads_time = 0;

	float robot_min_time_between_bursts = 2.f;
	float robot_max_time_between_bursts = 6.f;
	float robot_time_between_shots_in_burst = 0.1f;
	int robot_burst_size = 4;
	float robot_accuracy = 2.5f;

	bool robot_in_burst = false;
	float robot_time_till_next_burst = 20.0f;
	float robot_time_till_next_shot_in_burst = 0.0f;
	int robot_shots_until_end_of_burst = robot_burst_size;

	GLuint shadow_texture = 0;
	GLuint health_texture = 0;
	GLuint intro_image = 0;

	GLuint weapon_buffer = 0;
	GLuint weapon_buffer_col;
	GLuint weapon_buffer_depth;

	glm::vec2 player_pos = glm::vec2(0, 0);
	glm::vec2 move_vector = glm::vec2(0, 0);

	float last_fire_time = 0.0f;
	bool isADS = true;

	std::filesystem::path images_path;
	std::filesystem::path objects_path;

	// Main program
	GLuint main_program;
	GLuint skybox_program;
	GLuint shot_program;
	GLuint postprocess_program;
	GLuint healthbar_program;
	GLuint healthbar_vao;

	GLuint framebuffer_2;
	GLuint framebuffer_2_color;
	GLuint framebuffer_2_depth;

	GLuint skybox_vao;
	// TODO: feel free to add as many as you need/like

	// List of geometries used in the project
	std::vector<std::shared_ptr<Geometry>> geometries;
	// Shared pointers are pointers that automatically count how many times they are used. When there are 0 pointers to the object pointed
	// by shared_ptrs, the object is automatically deallocated. Consequently, we gain 3 main properties:
	// 1. Objects are not unnecessarily copied
	// 2. We don't have to track pointers
	// 3. We don't have to deallocate these geometries
	std::shared_ptr<Geometry> sphere;
	std::shared_ptr<Geometry> bunny;


public: class ShotLine {
public:
	glm::vec3 pos1;
	glm::vec3 pos2;
	glm::vec3 color;
	GLuint vao;
	GLuint vtx_vbo;
	std::vector<glm::vec3> vertices;
	float spawn_time;
	float max_time_alive;
	ShotLine(glm::vec3 pos1, glm::vec3 pos2, float aliveTime, glm::vec3 color, float thickness);
	void Draw();
};

public:class Object {
public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	int type = NONE;
	bool use_environmental_mapping = false;
	std::string name;
	float uv_scale = 1.0f;
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 BBOX_min = glm::vec3(0.0f);
	glm::vec3 BBOX_max = glm::vec3(0.0f);
	float fireCycle_pos = 0.0f;
	glm::vec3 fire_offset = glm::vec3(0.0f, 0.0f, 0.0f);
	GLuint color_texture = -1;
	GLuint normal_texture = -1;
	GLuint roughness_texture = -1;
	GLuint metalness_texture = -1;
	std::shared_ptr<Geometry> geometry_ptr = NULL;
	GLuint obj_ubo_id = 0;
	ObjectUBO obj_ubo = { .model_matrix = glm::mat4(1.0f),
					.ambient_color = glm::vec4(0.01f),
					.diffuse_color = glm::vec4(1.0f),
					.specular_color = glm::vec4(0.0f) };

	std::vector<std::shared_ptr<Object>> children;
	Object* parent = NULL;

	void Fire();


	Object() {};
	Object(const std::filesystem::path col, const std::filesystem::path normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries, ObjectUBO* ubo, std::string name);
	Object(GLuint col, GLuint normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries, ObjectUBO* ubo, std::string name);
	Object(GLuint col, GLuint normal, const std::filesystem::path _geometry, std::vector<std::shared_ptr<Geometry>>* geometries, std::string namee, glm::vec3 bbox_min, glm::vec3 bbox_max, int type);
	static GLuint load_texture_2d(const std::filesystem::path filename);
	void rotate_to_look(glm::vec3 lookDirection);
	void Draw(glm::vec3 lookDirection);
	void CalculateBBOX(std::filesystem::path pth);
	~Object();
private:
	void Cycle_fire();
	MyCamera* camera = camera;
};

	  std::vector<Object> objects;
	  enum obj_type { PLAYER, ENEMY, LEVEL, PISTOL, NONE, INVISIBLE_ENEMY, INVISIBLE };

	  std::vector<std::shared_ptr<ShotLine>> shotLines;

	  float robot_hp = 100.0f;
	  float player_hp = 100.0f;

	  Object pistol;
	  Object ground_obj;
	  Object cube_bbox_test_obj;
	  Object robot_top;
	  glm::vec3 player_bbox1 = glm::vec3(-0.5f, -1.0f, -0.5f);
	  glm::vec3 player_bbox2 = glm::vec3(0.5f, 1.0f, 0.5f);

	  GLuint skybox;

	  // UBOs
	  GLuint camera_buffer = 0;
	  CameraUBO camera_ubo;

	  GLuint light_buffer = 0;
	  LightUBO light_ubo;

	  GLuint objects_buffer = 0;
	  std::vector<ObjectUBO> objects_ubos;

	  // Lights
	  std::vector<LightUBO> lights;
	  GLuint lights_buffer = 0;

	  GLuint dir_light_buffer = 0;
	  LightUBO dir_light_ubo;

	  std::vector<glm::vec3> vibrantColors = {
		  glm::vec3(1.0f, 0.0f, 0.0f),   // Red
		  glm::vec3(0.0f, 1.0f, 0.0f),   // Green
		  glm::vec3(0.0f, 0.0f, 1.0f),   // Blue
		  glm::vec3(1.0f, 1.0f, 0.0f),   // Yellow
		  glm::vec3(1.0f, 0.0f, 1.0f),   // Magenta
		  glm::vec3(0.0f, 1.0f, 1.0f),   // Cyan
		  glm::vec3(1.0f, 0.5f, 0.0f),   // Orange
		  glm::vec3(0.0f, 1.0f, 0.5f),   // Lime
		  glm::vec3(0.5f, 0.0f, 1.0f),   // Purple
		  glm::vec3(0.5f, 1.0f, 0.0f)    // Chartreuse
	  };

	  // Textures


	  // ----------------------------------------------------------------------------
	  // Constructors & Destructors
	  // ----------------------------------------------------------------------------
public:
	/**
	 * Constructs a new @link Application with a custom width and height.
	 *
	 * @param 	initial_width 	The initial width of the window.
	 * @param 	initial_height	The initial height of the window.
	 * @param 	arguments	  	The command line arguments used to obtain the application directory.
	 */
	Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

	/** Destroys the {@link Application} and releases the allocated resources. */
	~Application() override;

	void parse_input(float delta);

	void on_mouse_move_custom(double x, double y);
	GLuint loadCubemap(std::vector<std::filesystem::path> faces);
	void Shoot(glm::vec3 shoot_origin, glm::vec3 shoot_direction, glm::vec3 color, int exclude_tag, float accuracy, float thickness, float ttl);
	bool checkCollisions(glm::vec3 target_pos, glm::vec3 BBOX_min, glm::vec3 BBOX_max);
	void SpawnShotLine(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, float thickness, float ttl);
	void importLevel();
	void buildLightsBuffer();

	// ----------------------------------------------------------------------------
	// Methods
	// ----------------------------------------------------------------------------

	/** @copydoc PV112Application::compile_shaders */
	void compile_shaders() override;

	void bindBuffers(GLuint model_matrix, bool camera, bool light, bool texture);

	/** @copydoc PV112Application::delete_shaders */
	void delete_shaders() override;

	/** @copydoc PV112Application::update */
	void update(float delta) override;

	/** @copydoc PV112Application::render */
	void render() override;

	/** @copydoc PV112Application::render_ui */
	void render_ui() override;

	// ----------------------------------------------------------------------------
	// Input Events
	// ----------------------------------------------------------------------------

	/** @copydoc PV112Application::on_resize */
	void on_resize(int width, int height) override;

	/** @copydoc PV112Application::on_mouse_move */
	void on_mouse_move(double x, double y) override;

	/** @copydoc PV112Application::on_mouse_button */
	void on_mouse_button(int button, int action, int mods) override;

	/** @copydoc PV112Application::on_key_pressed */
	void on_key_pressed(int key, int scancode, int action, int mods) override;
};

static Application* application;
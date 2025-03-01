#pragma once
#include <glm/glm.hpp>
#include <vector>

bool isCollidingSphereSphere(glm::vec3 pos1, glm::vec3 pos2, float radius1, float radius2);
bool isCollidingSphereBox(glm::vec3 pos1, glm::vec3 pos2, float radius, glm::vec4 bbox);
bool isCollidingBoxBox(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 bbox1_1, glm::vec3 bbox1_2, glm::vec3 bbox2_1, glm::vec3 bbox2_2);
glm::vec3 isCollidingLineBox(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 bbox1_1, glm::vec3 bbox1_2);
bool inline isPointInsideBBOX(glm::vec3 point, glm::vec3 bbox_min, glm::vec3 bbox_max);
bool isCollidingOnMove(glm::vec3 target_pos, glm::vec3 collider_pos, glm::vec3 BBOX1_min, glm::vec3 BBOX1_max,
    glm::vec3 BBOX2_min, glm::vec3 BBOX2_max);
